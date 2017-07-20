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

void FAnimNode_IKHumanoidLegTrace::Update(const FAnimationUpdateContext & Context)
{
	EvaluateGraphExposedInputs.Execute(Context);
}

void FAnimNode_IKHumanoidLegTrace::EvaluateComponentSpace(FComponentSpacePoseContext& Output)
{
	SCOPE_CYCLE_COUNTER(STAT_IKHumanoidLegTrace_Eval);

	USkeletalMeshComponent* SkelComp    = Output.AnimInstanceProxy->GetSkelMeshComponent();
	ACharacter* Character               = Cast<ACharacter>(SkelComp->GetOwner());
	const FBoneContainer& RequiredBones = Output.AnimInstanceProxy->GetRequiredBones();

	if (!IsValidToEvaluate(RequiredBones))
	{
		// IsValidToEvaluate checks if input pointers are null
		return;
	}
	
	FHumanoidIK::HumanoidIKLegTrace(Character, Output.Pose, Leg->Chain,
		PelvisBone->Bone, MaxPelvisAdjustSize, TraceData->TraceData, false);
}


bool FAnimNode_IKHumanoidLegTrace::IsValidToEvaluate(const FBoneContainer & RequiredBones)
{
	if (Leg == nullptr || PelvisBone == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid IK Leg Trace was not valid to evaluate -- a bone wrapper was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;
	}

	if (TraceData == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid IK Leg Trace was not valid to evaluate -- Trace data was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;
	}
		
	bool bValid = Leg->InitIfInvalid(RequiredBones);

	return bValid;
}


void FAnimNode_IKHumanoidLegTrace::Initialize(const FAnimationInitializeContext& Context)
{
	const FBoneContainer& RequiredBones = Context.AnimInstanceProxy->GetRequiredBones();
	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize Humanoid IK Leg Trace -- Leg invalid"));
#endif // ENABLE_IK_DEBUG

		return;
	}

	if (!Leg->InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize Humanoid IK Leg Trace"));
#endif // ENABLE_IK_DEBUG
	}
}


