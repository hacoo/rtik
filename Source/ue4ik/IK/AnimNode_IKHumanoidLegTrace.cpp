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

	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Could not evaluate Humanoid Leg IK Trace, Leg was null"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		return;
	}
	
	USkeletalMeshComponent* SkelComp    = Output.AnimInstanceProxy->GetSkelMeshComponent();
	USkeleton* Skeleton                 = Output.AnimInstanceProxy->GetSkeleton();
	const FBoneContainer& RequiredBones = Output.AnimInstanceProxy->GetRequiredBones();

	if (!IsValidToEvaluate(Skeleton, RequiredBones))
	{
		return;
	}
}


bool FAnimNode_IKHumanoidLegTrace::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid IK Leg Trace was not valid to evaluate -- Leg was null"));		
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


