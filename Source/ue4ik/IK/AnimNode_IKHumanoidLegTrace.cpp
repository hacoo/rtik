// Copyright(c) Henry Cooney 2017

#include "AnimNode_IKHumanoidLegTrace.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Runtime/AnimationCore/Public/TwoBoneIK.h"
#include "Utility/AnimUtil.h" 

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Leg IK Trace"), STAT_IKHumanoidLegTrace_Eval, STATGROUP_Anim);

void FAnimNode_IKHumanoidLegTrace::UpdateInternal(const FAnimationUpdateContext & Context)
{
	// Mark trace data as stale
	TraceData->bUpdatedThisTick = false;
}

void FAnimNode_IKHumanoidLegTrace::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, 
	TArray<FBoneTransform>& OutBoneTransforms) 
{
	SCOPE_CYCLE_COUNTER(STAT_IKHumanoidLegTrace_Eval);

	if (Leg == nullptr || PelvisBone == nullptr) 
	{
		return;
	}

	USkeletalMeshComponent* SkelComp    = Output.AnimInstanceProxy->GetSkelMeshComponent();
	ACharacter* Character               = Cast<ACharacter>(SkelComp->GetOwner());
	const FBoneContainer& RequiredBones = Output.AnimInstanceProxy->GetRequiredBones();

	FHumanoidIK::HumanoidIKLegTrace(Character, Output.Pose, Leg->Chain,
		PelvisBone->Bone, MaxPelvisAdjustSize, TraceData->TraceData, false);
	
	TraceData->bUpdatedThisTick = true;
}


bool FAnimNode_IKHumanoidLegTrace::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer & RequiredBones)
{
	if (Leg == nullptr || PelvisBone == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogRTIK, Warning, TEXT("IK Node Humanoid IK Leg Trace was not valid to evaluate -- a bone wrapper was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;
	}

	if (TraceData == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogRTIK, Warning, TEXT("IK Node Humanoid IK Leg Trace was not valid to evaluate -- Trace data was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;
	}
		
	bool bValid = Leg->InitIfInvalid(RequiredBones);

	return bValid;
}


void FAnimNode_IKHumanoidLegTrace::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialize Humanoid IK Leg Trace -- Leg invalid"));
#endif // ENABLE_IK_DEBUG

		return;
	}

	if (!Leg->InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialize Humanoid IK Leg Trace"));
#endif // ENABLE_IK_DEBUG
	}
}


