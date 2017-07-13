// Copyright (c) Henry Cooney 2017

#include "AnimNode_HumanoidPelvisHeightAdjustment.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "IK/IK.h"
#include "HumanoidIK.h"

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Pelvis Height Adjust Eval"), STAT_HumanoidPelvisHeightAdjust_Eval, STATGROUP_Anim);


void FAnimNode_HumanoidPelvisHeightAdjustment::UpdateInternal(const FAnimationUpdateContext & Context)
{
	DeltaTime = Context.GetDeltaTime();
}

void FAnimNode_HumanoidPelvisHeightAdjustment::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_HumanoidPelvisHeightAdjust_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

	USkeletalMeshComponent* SkelComp = Output.AnimInstanceProxy->GetSkelMeshComponent();
	ACharacter* Character = Cast<ACharacter>(SkelComp->GetOwner());
	FHumanoidIKTraceData LeftTraceData;
	FHumanoidIK::HumanoidIKLegTrace(Character, Output.Pose, LeftLeg, PelvisBone, MaxPelvisAdjustHeight, LeftTraceData, true);	
}

bool FAnimNode_HumanoidPelvisHeightAdjustment::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	bool bValid = LeftLeg.InitIfInvalid(RequiredBones)
		&& RightLeg.InitIfInvalid(RequiredBones)
		&& PelvisBone.InitIfInvalid(RequiredBones);

#if ENABLE_IK_DEBUG_VERBOSE
	if (!bValid)
	{
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid Pelvis Height Adjustment was not valid to evaluate"));
	}
#endif // ENABLE_ANIM_DEBUG

	return bValid;
}

void FAnimNode_HumanoidPelvisHeightAdjustment::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	UE_LOG(LogIK, Warning, TEXT("Initializing biped leg IK..."));
	if (!RightLeg.InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize right leg for biped hip adjustment"));
#endif // ENABLE_IK_DEBUG
	}

	if (!LeftLeg.InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize left leg for biped hip adjustment"));
#endif // ENABLE_IK_DEBUG
	}

	if (!PelvisBone.Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize pelvis bone for biped hip adjustment"));
#endif // ENABLE_IK_DEBUG
	}	
}


