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
	FAnimationRuntime::ConvertBoneSpaceTransformToCS(Output.AnimInstanceProxy->GetComponentTransform(), Output.Pose, CSEffectorTransform, EffectorTransformBone.GetCompactPoseIndex(BoneContainer), EffectorTransformSpace);
	
	const FVector CSEffectorLocation = CSEffectorTransform.GetLocation();

#if WITH_EDITOR
	CachedEffectorCSTransform = CSEffectorTransform;
#endif

	// Gather all bone indices between root and tip.
	TArray<FCompactPoseBoneIndex> BoneIndices;
	
	{
		const FCompactPoseBoneIndex RootIndex = RootBone.GetCompactPoseIndex(BoneContainer);
		FCompactPoseBoneIndex BoneIndex = TipBone.GetCompactPoseIndex(BoneContainer);
		do
		{
			BoneIndices.Insert(BoneIndex, 0);
			BoneIndex = Output.Pose.GetPose().GetParentBoneIndex(BoneIndex);
		} while (BoneIndex != RootIndex);
		BoneIndices.Insert(BoneIndex, 0);
	}

	// Maximum length of skeleton segment at full extension
	float MaximumReach = 0;

	// Gather transforms
	const int32 NumTransforms = BoneIndices.Num();

	// Gather chain links. These are non zero length bones.
	TArray<FRangeLimitedFABRIKChainLink> Chain;
	Chain.Reserve(NumTransforms);

	// Start with Root Bone
	{
		const FCompactPoseBoneIndex& RootBoneIndex = BoneIndices[0];
		const FTransform& BoneCSTransform          = Output.Pose.GetComponentSpaceTransform(RootBoneIndex);

		Chain.Add(FRangeLimitedFABRIKChainLink(0.f, RootBoneIndex, BoneCSTransform));
	}

	// Go through remaining transforms
	for (int32 i = 1; i < NumTransforms; ++i)
	{
		const FCompactPoseBoneIndex& BoneIndex = BoneIndices[i];
		const FTransform& BoneCSTransform      = Output.Pose.GetComponentSpaceTransform(BoneIndex);

		// Calculate the combined length of this segment of skeleton
		const float BoneLength = FVector::Dist(BoneCSTransform.GetLocation(), Chain[i-1].BoneCSTransform.GetLocation());

		// No need to check for zero-length bones -- already checked in initiallization of RangeLimitedIKChain
		Chain.Add(FRangeLimitedFABRIKChainLink(BoneLength, BoneIndex, BoneCSTransform));
		MaximumReach += BoneLength;
	}

	bool bBoneLocationUpdated = false;
	const float RootToTargetDistSq = FVector::DistSquared(Chain[0].BoneCSTransform.GetLocation(), CSEffectorLocation);
	const int32 NumChainLinks = Chain.Num();

	// FABRIK algorithm - bone translation calculation
	// If the effector is further away than the distance from root to tip, simply move all bones in a line from root to effector location
	if (RootToTargetDistSq > FMath::Square(MaximumReach))
	{
		for (int32 LinkIndex = 1; LinkIndex < NumChainLinks; LinkIndex++)
		{
			FRangeLimitedFABRIKChainLink const & ParentLink = Chain[LinkIndex - 1];
			FRangeLimitedFABRIKChainLink & CurrentLink = Chain[LinkIndex];
			CurrentLink.BoneCSTransform.SetLocation(ParentLink.BoneCSTransform.GetLocation() + 
				(CSEffectorLocation - ParentLink.BoneCSTransform.GetLocation()).GetUnsafeNormal() * CurrentLink.Length);
		}
		bBoneLocationUpdated = true;
	}
	else // Effector is within reach, calculate bone translations to position tip at effector location
	{
		const int32 TipBoneLinkIndex = NumChainLinks - 1;

		// Check distance between tip location and effector location
		float Slop = FVector::Dist(Chain[TipBoneLinkIndex].BoneCSTransform.GetLocation(), CSEffectorLocation);
		if (Slop > Precision)
		{
			// Set tip bone at end effector location.
			Chain[TipBoneLinkIndex].BoneCSTransform.SetLocation(CSEffectorLocation);

			int32 IterationCount = 0;
			while ((Slop > Precision) && (IterationCount++ < MaxIterations))
			{
				// "Forward Reaching" stage - adjust bones from end effector.
				for (int32 LinkIndex = TipBoneLinkIndex - 1; LinkIndex > 0; LinkIndex--)
				{
					FRangeLimitedFABRIKChainLink& CurrentLink = Chain[LinkIndex];
					FRangeLimitedFABRIKChainLink& ChildLink = Chain[LinkIndex + 1];

					CurrentLink.BoneCSTransform.SetLocation(ChildLink.BoneCSTransform.GetLocation() +
						(CurrentLink.BoneCSTransform.GetLocation() - ChildLink.BoneCSTransform.GetLocation()).GetUnsafeNormal() * ChildLink.Length);

					UpdateParentRotation(CurrentLink, ChildLink, Output.Pose);
				}

				// "Backward Reaching" stage - adjust bones from root.
				for (int32 LinkIndex = 1; LinkIndex < TipBoneLinkIndex; LinkIndex++)
				{
					FRangeLimitedFABRIKChainLink& ParentLink = Chain[LinkIndex - 1];
					FRangeLimitedFABRIKChainLink& CurrentLink = Chain[LinkIndex];

					CurrentLink.BoneCSTransform.SetLocation(ParentLink.BoneCSTransform.GetLocation() +
						(CurrentLink.BoneCSTransform.GetLocation() - ParentLink.BoneCSTransform.GetLocation()).GetUnsafeNormal() * CurrentLink.Length);

					UpdateParentRotation(ParentLink, CurrentLink, Output.Pose);
					
				}

				// Re-check distance between tip location and effector location
				// Since we're keeping tip on top of effector location, check with its parent bone.
				Slop = FMath::Abs(Chain[TipBoneLinkIndex].Length -
					FVector::Dist(Chain[TipBoneLinkIndex - 1].BoneCSTransform.GetLocation(), CSEffectorLocation));
			}

			// Place tip bone based on how close we got to target.
			{
				FRangeLimitedFABRIKChainLink const & ParentLink = Chain[TipBoneLinkIndex - 1];
				FRangeLimitedFABRIKChainLink & CurrentLink = Chain[TipBoneLinkIndex];

				CurrentLink.BoneCSTransform.SetLocation(ParentLink.BoneCSTransform.GetLocation() + 
					(CurrentLink.BoneCSTransform.GetLocation() - ParentLink.BoneCSTransform.GetLocation()).GetUnsafeNormal() * CurrentLink.Length);
			}

			bBoneLocationUpdated = true;
		}
	}

	// Special handling for tip bone's rotation.
	const int32 TipBoneIndex = Chain.Num() - 1;
	switch (EffectorRotationSource)
	{
	case BRS_KeepLocalSpaceRotation:
		if (Chain.Num() > 1)
		{
			Chain[TipBoneIndex].BoneCSTransform =
				Output.Pose.GetLocalSpaceTransform(BoneIndices[TipBoneIndex]) *
				Chain[TipBoneIndex - 1].BoneCSTransform;
		}
		break;
	case BRS_CopyFromTarget:
		Chain[TipBoneIndex].BoneCSTransform.SetRotation(CSEffectorTransform.GetRotation());
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
		size_t NumLinks = Chain.Num();
		OutBoneTransforms.Reserve(NumLinks);

		for (size_t i = 0; i < NumLinks; ++i)
		{
			int32 Index = Chain[i].BoneIndex.GetInt();
			OutBoneTransforms.Add(FBoneTransform(Chain[i].BoneIndex, Chain[i].BoneCSTransform));
		}
	}
}

void FAnimNode_RangeLimitedFabrik::EnforceROMConstraint(FCSPose<FCompactPose>& Pose, TArray<FRangeLimitedFABRIKChainLink>& Chain, 
	FIKBone& ChildBone, int32 ChildIndex)
{
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
	

/*
	else if (ChildBone.ConstraintMode == EIKROMConstraintMode::IKROM_Pitch_And_Yaw)
	{
		
		
	}
*/
	


}

void FAnimNode_RangeLimitedFabrik::UpdateParentRotation(FRangeLimitedFABRIKChainLink& ParentLink,
	const FRangeLimitedFABRIKChainLink& ChildLink, FCSPose<FCompactPose>& Pose)
{
	
	// Calculate pre-translation vector between this bone and child
	FTransform OldParentTransform = Pose.GetComponentSpaceTransform(ParentLink.BoneIndex);
	FTransform OldChildTransform = Pose.GetComponentSpaceTransform(ChildLink.BoneIndex);
	FVector OldDir = (OldChildTransform.GetLocation() - OldParentTransform.GetLocation()).GetUnsafeNormal();

	// Get vector from the post-translation bone to it's child
	FVector NewDir = (ChildLink.BoneCSTransform.GetLocation() -
		ParentLink.BoneCSTransform.GetLocation()).GetUnsafeNormal();
	
	// Calculate axis of rotation from pre-translation vector to post-translation vector
	FVector RotationAxis = FVector::CrossProduct(OldDir, NewDir).GetSafeNormal();
	float RotationAngle = FMath::Acos(FVector::DotProduct(OldDir, NewDir));
	FQuat DeltaRotation = FQuat(RotationAxis, RotationAngle);
	// We're going to multiply it, in order to not have to re-normalize the final quaternion, it has to be a unit quaternion.
	checkSlow(DeltaRotation.IsNormalized());
	
	// Calculate absolute rotation and set it
	ParentLink.BoneCSTransform.SetRotation(DeltaRotation * OldParentTransform.GetRotation());
	ParentLink.BoneCSTransform.NormalizeRotation();
}


bool FAnimNode_RangeLimitedFabrik::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
/*
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
*/

	// Allow evaluation if all parameters are initialized and TipBone is child of RootBone
	return
		(
			TipBone.IsValid(RequiredBones)
			&& RootBone.IsValid(RequiredBones)
			&& Precision > 0
			&& RequiredBones.BoneIsChildOf(TipBone.BoneIndex, RootBone.BoneIndex)
	//		&& IKChain->IsValid(RequiredBones)
		);
}

void FAnimNode_RangeLimitedFabrik::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
/*
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
	
	TipBone  = IKChain->Chain[0].BoneRef;
	RootBone = IKChain->Chain[NumBones - 1].BoneRef;
*/
	
	TipBone.Initialize(RequiredBones);
	RootBone.Initialize(RequiredBones);
	EffectorTransformBone.Initialize(RequiredBones);
}

void FAnimNode_RangeLimitedFabrik::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugData.AddDebugItem(DebugLine);
	ComponentPose.GatherDebugData(DebugData);
}
