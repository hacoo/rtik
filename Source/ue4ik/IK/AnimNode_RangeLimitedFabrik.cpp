// Copyright(c) Henry Cooney 2017

#include "AnimNode_RangeLimitedFabrik.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "Components/SkeletalMeshComponent.h"

FAnimNode_RangeLimitedFabrik::FAnimNode_RangeLimitedFabrik()
	: EffectorTransform(FTransform::Identity)
	, EffectorTransformSpace(BCS_ComponentSpace)
	, EffectorRotationSource(BRS_KeepLocalSpaceRotation)
	, Precision(1.f)
	, MaxIterations(10)
	, bEnableDebugDraw(false)
{
}

FVector FAnimNode_RangeLimitedFabrik::GetCurrentLocation(FCSPose<FCompactPose>& MeshBases, const FCompactPoseBoneIndex& BoneIndex)
{
	return MeshBases.GetComponentSpaceTransform(BoneIndex).GetLocation();
}

void FAnimNode_RangeLimitedFabrik::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

	// Update EffectorLocation if it is based off a bone position
	FTransform CSEffectorTransform = EffectorTransform;
	FAnimationRuntime::ConvertBoneSpaceTransformToCS(Output.AnimInstanceProxy->GetComponentTransform(),
		Output.Pose, CSEffectorTransform, EffectorTransformBone.GetCompactPoseIndex(BoneContainer), 
		EffectorTransformSpace);
	
	FVector CSEffectorLocation = CSEffectorTransform.GetLocation();

#if WITH_EDITOR
	CachedEffectorCSTransform = CSEffectorTransform;
#endif	

	int32 NumChainLinks = IKChain->Chain.Num();
	if (NumChainLinks < 2)
	{
		return;
	}

	// Maximum length of skeleton segment at full extension
	float MaximumReach = 0;

	// Gather bone transforms
	TArray<FTransform> CSTransforms;
	CSTransforms.Reserve(NumChainLinks);	
	for (int32 i = 0; i < NumChainLinks; ++i)
	{
		CSTransforms.Add(Output.Pose.GetComponentSpaceTransform(IKChain->Chain[i].BoneIndex));
	}

	// Gather bone lengths
	TArray<float> BoneLengths;
	BoneLengths.Reserve(NumChainLinks);
	BoneLengths.Add(0.0f);
	for (int32 i = 1; i < NumChainLinks; ++i)
	{
		BoneLengths.Add(FVector::Dist(CSTransforms[i - 1].GetLocation(), CSTransforms[i].GetLocation()));
		MaximumReach  += BoneLengths[i];
	}

	bool bBoneLocationUpdated = false;
	float RootToTargetDistSq = FVector::DistSquared(CSTransforms[0].GetLocation(), CSEffectorLocation);

	// FABRIK algorithm - bone translation calculation
	// If the effector is further away than the distance from root to tip, simply move all bones in a line from root to effector location
	if (RootToTargetDistSq > FMath::Square(MaximumReach))
	{
		for (int32 LinkIndex = 1; LinkIndex < NumChainLinks; LinkIndex++)
		{
			FTransform& ParentLink = CSTransforms[LinkIndex - 1];
			FTransform& CurrentLink = CSTransforms[LinkIndex];
			CurrentLink.SetLocation(ParentLink.GetLocation() +
				(CSEffectorLocation - ParentLink.GetLocation()).GetUnsafeNormal() *
				BoneLengths[LinkIndex]);
		}
		bBoneLocationUpdated = true;
	}
	else // Effector is within reach, calculate bone translations to position tip at effector location
	{
		int32 TipBoneLinkIndex = NumChainLinks - 1;

		// Check distance between tip location and effector location
		float Slop = FVector::Dist(CSTransforms[TipBoneLinkIndex].GetLocation(), CSEffectorLocation);
		if (Slop > Precision)
		{
			// Set tip bone at end effector location.
			CSTransforms[TipBoneLinkIndex].SetLocation(CSEffectorLocation);

			int32 IterationCount = 0;
			while ((Slop > Precision) && (IterationCount++ < MaxIterations))
			{
				// "Forward Reaching" stage - adjust bones from end effector.
				for (int32 LinkIndex = TipBoneLinkIndex - 1; LinkIndex > 0; LinkIndex--)
				{
					FTransform& CurrentLink = CSTransforms[LinkIndex];
					FTransform& ChildLink   = CSTransforms[LinkIndex + 1];

					CurrentLink.SetLocation(ChildLink.GetLocation() +
						(CurrentLink.GetLocation() - ChildLink.GetLocation()).GetUnsafeNormal() *
						BoneLengths[LinkIndex]);

					// UpdateParentRotation(CurrentLink, IKChain->Chain[LinkIndex],
						// ChildLink, IKChain->Chain[LinkIndex + 1],
						// Output.Pose);
				}

				// "Backward Reaching" stage - adjust bones from root.
				for (int32 LinkIndex = 1; LinkIndex < TipBoneLinkIndex; LinkIndex++)
				{
					FTransform& ParentLink = CSTransforms[LinkIndex - 1];
					FTransform& CurrentLink = CSTransforms[LinkIndex];

					CurrentLink.SetLocation(ParentLink.GetLocation() +
						(CurrentLink.GetLocation() - ParentLink.GetLocation()).GetUnsafeNormal() *
						BoneLengths[LinkIndex]);

					// UpdateParentRotation(ParentLink, IKChain->Chain[LinkIndex - 1],
						// CurrentLink, IKChain->Chain[LinkIndex],
						// Output.Pose);
				}

				// Re-check distance between tip location and effector location
				// Since we're keeping tip on top of effector location, check with its parent bone.
				Slop = FMath::Abs(BoneLengths[TipBoneLinkIndex] - 
					FVector::Dist(CSTransforms[TipBoneLinkIndex - 1].GetLocation(), CSEffectorLocation));
			}

			// Place tip bone based on how close we got to target.
			{
				FTransform& ParentLink  = CSTransforms[TipBoneLinkIndex - 1];
				FTransform& CurrentLink = CSTransforms[TipBoneLinkIndex];

				CurrentLink.SetLocation(ParentLink.GetLocation() +
					(CurrentLink.GetLocation() - ParentLink.GetLocation()).GetUnsafeNormal() *
					BoneLengths[TipBoneLinkIndex]);

				//UpdateParentRotation(ParentLink, IKChain->Chain[TipBoneLinkIndex - 1],
				//CurrentLink, IKChain->Chain[TipBoneLinkIndex],
				//Output.Pose);
			}

			bBoneLocationUpdated = true;
		}
	}

	// Update bone rotations
	if (bBoneLocationUpdated) 
	{
		for (int32 LinkIndex = 0; LinkIndex < CSTransforms.Num() - 1; ++LinkIndex)
		{
			UpdateParentRotation(CSTransforms[LinkIndex], IKChain->Chain[LinkIndex],
				CSTransforms[LinkIndex + 1], IKChain->Chain[LinkIndex + 1],
				Output.Pose);
		}
	}

	// Special handling for tip bone's rotation.
	int32 TipBoneIndex = CSTransforms.Num() - 1;
	switch (EffectorRotationSource)
	{
	case BRS_KeepLocalSpaceRotation:
		if (CSTransforms.Num() > 1)
		{
			CSTransforms[TipBoneIndex] = Output.Pose.GetLocalSpaceTransform(IKChain->Chain[TipBoneIndex].BoneIndex) *
				CSTransforms[TipBoneIndex - 1];
		}
		break;
	case BRS_CopyFromTarget:
		CSTransforms[TipBoneIndex].SetRotation(CSEffectorTransform.GetRotation());
		break;
	case BRS_KeepComponentSpaceRotation:
		// Don't change the orientation at all
		break;
	default:
		break;
	}

	// Commit the changes, if there were any
	if (bBoneLocationUpdated)
	{
		int32 NumLinks = CSTransforms.Num();
		OutBoneTransforms.Reserve(NumLinks);

		for (int32 i = 0; i < NumLinks; ++i)
		{
			OutBoneTransforms.Add(FBoneTransform(IKChain->Chain[i].BoneIndex, CSTransforms[i]));
		}
	}
}

void FAnimNode_RangeLimitedFabrik::EnforceROMConstraint(FCSPose<FCompactPose>& Pose, 
	FIKBone& ChildBone, int32 ChildIndex)
{
	/*
	if (ChildBone.ConstraintMode == EIKROMConstraintMode::IKROM_No_Constraint)
	{
		return;
	}
	FRangeLimitedFABRIKChainLink& ChildLink = Chain[ChildIndex];
	FVector ChildLoc = ChildLink.BoneCSTransform.GetLocation();

	FVector ChildDirection;
	FVector ParentDirection;

	// Step 1: determine the forward directions of the parent and child
	if (ChildBone.bUseParentBoneDirection)
	{
		// Find the parent location -- either by looking at the chain, or going to the skeletal parent (if root)
		FTransform ParentTransform;
		if (ChildIndex < 1)
		{
			if (ChildLink.BoneIndex < 1)
			{
				// This bone is the root of the entire skeleton! So just use its default transform.
				ParentTransform = Pose.GetComponentSpaceTransform(ChildLink.BoneIndex);
			}
			else
			{
				// At root of chain -- look at skeleton instead
				FCompactPoseBoneIndex ParentBoneIndex = Pose.GetPose().GetParentBoneIndex(ChildLink.BoneIndex);
				ParentTransform = Pose.GetComponentSpaceTransform(ParentBoneIndex);
			}
		} 
		else
		{
			// Use the parent link transform
			ParentTransform = Chain[ChildIndex - 1].BoneCSTransform;
		}

	}
	
*/

/*
	else if (ChildBone.ConstraintMode == EIKROMConstraintMode::IKROM_Pitch_And_Yaw)
	{
		
		
	}
*/
}

void FAnimNode_RangeLimitedFabrik::UpdateParentRotation(FTransform& ParentTransform, const FIKBone& ParentBone,
	FTransform& ChildTransform, const FIKBone& ChildBone, FCSPose<FCompactPose>& Pose) const
{
	
	// Calculate pre-translation vector between this bone and child
	FTransform OldParentTransform = Pose.GetComponentSpaceTransform(ParentBone.BoneIndex);
	FTransform OldChildTransform = Pose.GetComponentSpaceTransform(ChildBone.BoneIndex);
	FVector OldDir = (OldChildTransform.GetLocation() - OldParentTransform.GetLocation()).GetUnsafeNormal();

	// Get vector from the post-translation bone to it's child
	FVector NewDir = (ChildTransform.GetLocation() -
		ParentTransform.GetLocation()).GetUnsafeNormal();
	
	// Calculate axis of rotation from pre-translation vector to post-translation vector
	FVector RotationAxis = FVector::CrossProduct(OldDir, NewDir).GetSafeNormal();
	float RotationAngle = FMath::Acos(FVector::DotProduct(OldDir, NewDir));
	FQuat DeltaRotation = FQuat(RotationAxis, RotationAngle);
	// We're going to multiply it, in order to not have to re-normalize the final quaternion, it has to be a unit quaternion.
	checkSlow(DeltaRotation.IsNormalized());
	
	// Calculate absolute rotation and set it
	ParentTransform.SetRotation(DeltaRotation * OldParentTransform.GetRotation());
	ParentTransform.NormalizeRotation();
}


bool FAnimNode_RangeLimitedFabrik::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{

	if (IKChain == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("AnimNode_RangeLimitedFabrik was not valid to evaluate -- an input wrapper object was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;
	}

	if (IKChain->Chain.Num() < 2)
	{
		return false;
	}

	// Allow evaluation if all parameters are initialized and TipBone is child of RootBone
	return
		(
			Precision > 0
			&& IKChain->IsValid(RequiredBones)
		);
}

void FAnimNode_RangeLimitedFabrik::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{

	if (IKChain == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize FAnimNode_RangeLimitedFabrik -- An input wrapper object was null"));
#endif // ENABLE_IK_DEBUG
		return;
	}

	IKChain->InitIfInvalid(RequiredBones);
	size_t NumBones = IKChain->Chain.Num();

	if (NumBones < 2)
	{
		return;
	}
	
	EffectorTransformBone = IKChain->Chain[NumBones - 1].BoneRef;
	EffectorTransformBone.Initialize(RequiredBones);
}

void FAnimNode_RangeLimitedFabrik::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugData.AddDebugItem(DebugLine);
	ComponentPose.GatherDebugData(DebugData);
}
