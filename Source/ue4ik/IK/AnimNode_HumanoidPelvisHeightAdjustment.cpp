// Copyright (c) Henry Cooney 2017

#include "AnimNode_HumanoidPelvisHeightAdjustment.h"
#include "IK.h"

void FAnimNode_HumanoidPelvisHeightAdjustment::UpdateInternal(const FAnimationUpdateContext & Context)
{
	FAnimNode_SkeletalControlBase::UpdateInternal(Context);
	DeltaTime = Context.GetDeltaTime();
}

void FAnimNode_HumanoidPelvisHeightAdjustment::EvaluateComponentSpaceInternal(FComponentSpacePoseContext & Context)
{
}

void FAnimNode_HumanoidPelvisHeightAdjustment::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{
}

bool FAnimNode_HumanoidPelvisHeightAdjustment::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	return bInitSuccess;
}

void FAnimNode_HumanoidPelvisHeightAdjustment::InitializeBoneReferences(const FBoneContainer & RequiredBones)
{
	bInitSuccess = true;
	if (!RightLeg.InitAndAssignBones(RequiredBones))
	{
		UE_LOG(LogIK, Warning, TEXT("Could not initialize right leg for biped hip adjustment"));
		bInitSuccess = false;
	}

	if (!LeftLeg.InitAndAssignBones(RequiredBones))
	{
		UE_LOG(LogIK, Warning, TEXT("Could not initialize left leg for biped hip adjustment"));
		bInitSuccess = false;
	}

	if (!PelvisBone.Init(RequiredBones))
	{
		UE_LOG(LogIK, Warning, TEXT("Could not initialize pelvis bone for biped hip adjustment"));
		bInitSuccess = false;
	}
}
