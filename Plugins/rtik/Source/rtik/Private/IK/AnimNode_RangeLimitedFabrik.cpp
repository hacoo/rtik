// Copyright(c) Henry Cooney 2017

#include "rtik.h"
#include "AnimNode_RangeLimitedFabrik.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "Components/SkeletalMeshComponent.h"
#include "IK/RangeLimitedFABRIK.h"
#include "Utility/DebugDrawUtil.h"

DECLARE_CYCLE_STAT(TEXT("IK Range Limited FABRIK"), STAT_RangeLimitedFabrik_Eval, STATGROUP_Anim);

void FAnimNode_RangeLimitedFabrik::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_RangeLimitedFabrik_Eval);
	
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
	check(OutBoneTransforms.Num() == 0);

	int32 NumChainLinks = IKChain->Chain.Num();
	if (NumChainLinks < 2)
	{
		return;
	}

	// Maximum length of skeleton segment at full extension
	float MaximumReach = 0;

	// Gather bone transforms and constraints
	TArray<FTransform> SourceCSTransforms;
	TArray<FIKBoneConstraint*> Constraints;
	SourceCSTransforms.Reserve(NumChainLinks);
	Constraints.Reserve(NumChainLinks);
	for (int32 i = 0; i < NumChainLinks; ++i)
	{
		SourceCSTransforms.Add(Output.Pose.GetComponentSpaceTransform(IKChain->Chain[i].BoneIndex));
		Constraints.Add(IKChain->Chain[i].GetConstraint());
	}

	TArray<FTransform> DestCSTransforms;

	ACharacter* Character = Cast<ACharacter>(Output.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner());
	bool bBoneLocationUpdated = false;

	if (SolverMode == ERangeLimitedFABRIKSolverMode::RLF_Normal)
	{
		bBoneLocationUpdated = FRangeLimitedFABRIK::SolveRangeLimitedFABRIK(
			SourceCSTransforms,
			Constraints,
			CSEffectorTransform.GetLocation(),
			DestCSTransforms,
			MaxRootDragDistance,
			RootDragStiffness,
			Precision,
			MaxIterations,
			Character
		);
	}
	else if (SolverMode == ERangeLimitedFABRIKSolverMode::RLF_ClosedLoop)
	{
		bBoneLocationUpdated = FRangeLimitedFABRIK::SolveClosedLoopFABRIK(
			SourceCSTransforms,
			Constraints,
			CSEffectorTransform.GetLocation(),
			DestCSTransforms,
			MaxRootDragDistance,
			RootDragStiffness,
			Precision,
			MaxIterations,
			Character
		);
	}

	// Special handling for tip bone's rotation.
	int32 TipBoneIndex = NumChainLinks - 1;
	switch (EffectorRotationSource)
	{
	case BRS_KeepLocalSpaceRotation:
		if (NumChainLinks > 1)
		{
			DestCSTransforms[TipBoneIndex] = Output.Pose.GetLocalSpaceTransform(IKChain->Chain[TipBoneIndex].BoneIndex) *
				DestCSTransforms[TipBoneIndex - 1];
		}
		break;
	case BRS_CopyFromTarget:
		DestCSTransforms[TipBoneIndex].SetRotation(CSEffectorTransform.GetRotation());
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
		OutBoneTransforms.Reserve(NumChainLinks);

		for (int32 i = 0; i < NumChainLinks; ++i)
		{
			OutBoneTransforms.Add(FBoneTransform(IKChain->Chain[i].BoneIndex, DestCSTransforms[i]));
		}
	}


#if WITH_EDITOR

	if (bEnableDebugDraw)
	{
		USkeletalMeshComponent* SkelComp = Output.AnimInstanceProxy->GetSkelMeshComponent();
		UWorld* World = SkelComp->GetWorld();
		FMatrix ToWorld = SkelComp->GetComponentToWorld().ToMatrixNoScale();

		if (SolverMode == ERangeLimitedFABRIKSolverMode::RLF_Normal)
		{
			// Draw chain before adjustment, in yellow
			for (int32 i = 0; i < NumChainLinks - 1; ++i)
			{
				FVector ParentLoc = ToWorld.TransformPosition(SourceCSTransforms[i].GetLocation());
				FVector ChildLoc = ToWorld.TransformPosition(SourceCSTransforms[i + 1].GetLocation());
				FDebugDrawUtil::DrawLine(World, ParentLoc, ChildLoc, FColor(255, 255, 0));
				FDebugDrawUtil::DrawSphere(World, ChildLoc, FColor(255, 255, 0), 3.0f);
			}

			// Draw root drag radius
			if (MaxRootDragDistance > KINDA_SMALL_NUMBER)
			{
				FDebugDrawUtil::DrawSphere(World,
					ToWorld.TransformPosition(SourceCSTransforms[0].GetLocation()),
					FColor(255, 0, 0), MaxRootDragDistance, 12, -1.0f, 0.5f);
			}

			// Draw chain after adjustment, in cyan
			for (int32 i = 0; i < NumChainLinks - 1; ++i)
			{
				FVector ParentLoc = ToWorld.TransformPosition(DestCSTransforms[i].GetLocation());
				FVector ChildLoc = ToWorld.TransformPosition(DestCSTransforms[i + 1].GetLocation());
				FDebugDrawUtil::DrawLine(World, ParentLoc, ChildLoc, FColor(0, 255, 255));
				FDebugDrawUtil::DrawSphere(World, ChildLoc, FColor(0, 255, 255), 3.0f);
			}
		}
		else if (SolverMode == ERangeLimitedFABRIKSolverMode::RLF_ClosedLoop)
		{
			// Draw chain before adjustment, in yellow
			for (int32 i = 0; i < NumChainLinks - 1; ++i)
			{
				FVector ParentLoc = ToWorld.TransformPosition(SourceCSTransforms[i].GetLocation());
				FVector ChildLoc = ToWorld.TransformPosition(SourceCSTransforms[i + 1].GetLocation());
				FDebugDrawUtil::DrawLine(World, ParentLoc, ChildLoc, FColor(255, 255, 0));
				FDebugDrawUtil::DrawSphere(World, ChildLoc, FColor(255, 255, 0), 3.0f);
			}

			FDebugDrawUtil::DrawLine(World,
				ToWorld.TransformPosition(SourceCSTransforms[0].GetLocation()),
				ToWorld.TransformPosition(SourceCSTransforms[NumChainLinks-1].GetLocation()),
				FColor(255, 255, 0));

			// Draw root drag radius
			if (MaxRootDragDistance > KINDA_SMALL_NUMBER)
			{
				FDebugDrawUtil::DrawSphere(World,
					ToWorld.TransformPosition(SourceCSTransforms[0].GetLocation()),
					FColor(255, 0, 0), MaxRootDragDistance, 12, -1.0f, 0.5f);
			}

			// Draw chain after adjustment, in cyan
			for (int32 i = 0; i < NumChainLinks - 1; ++i)
			{
				FVector ParentLoc = ToWorld.TransformPosition(DestCSTransforms[i].GetLocation());
				FVector ChildLoc = ToWorld.TransformPosition(DestCSTransforms[i + 1].GetLocation());
				FDebugDrawUtil::DrawLine(World, ParentLoc, ChildLoc, FColor(0, 255, 255));
				FDebugDrawUtil::DrawSphere(World, ChildLoc, FColor(0, 255, 255), 3.0f);
			}

			FDebugDrawUtil::DrawLine(World,
				ToWorld.TransformPosition(DestCSTransforms[0].GetLocation()),
				ToWorld.TransformPosition(DestCSTransforms[NumChainLinks-1].GetLocation()),
				FColor(0, 255, 255));	
		}
	}

#endif


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
		UE_LOG(LogRTIK, Warning, TEXT("AnimNode_RangeLimitedFabrik was not valid to evaluate -- an input wrapper object was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;
	}

	if (IKChain->Chain.Num() < 2)
	{
		return false;
	}

	// Allow evaluation if all parameters are initialized and TipBone is child of RootBone
	return (
		Precision > 0
		&& IKChain->IsValid(RequiredBones)
		);
}

void FAnimNode_RangeLimitedFabrik::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{

	if (IKChain == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialize FAnimNode_RangeLimitedFabrik -- An input wrapper object was null"));
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
