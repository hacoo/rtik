// Copyright(c) Henry Cooney 2017

#include "AnimNode_IKHumanoidLegTrace.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Runtime/AnimationCore/Public/TwoBoneIK.h"
#include "Utility/AnimUtil.h" 

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

void FAnimNode_IKHumanoidLegTrace::Update(const FAnimationUpdateContext & Context)
{
	EvaluateGraphExposedInputs.Execute(Context);
}

void FAnimNode_IKHumanoidLegTrace::EvaluateComponentSpace(FComponentSpacePoseContext& Output)
{
	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Could not evaluate Humanoid Leg IK, Leg was null"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		return;
	}
}

/*
bool FAnimNode_IKHumanoidLegTrace::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid IK Leg was not valid to evaluate -- Leg was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;
	}
	
	bool bValid = Leg->InitIfInvalid(RequiredBones);

#if ENABLE_IK_DEBUG_VERBOSE
	if (!bValid)
	{
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid IK Leg was not valid to evaluate"));
	}
#endif // ENABLE_ANIM_DEBUG

	
	bValid &= FabrikSolver.IsValidToEvaluate(Skeleton, RequiredBones);
#if ENABLE_IK_DEBUG_VERBOSE
	if (!bValid)
	{
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid IK Leg -- internal FABRIK solver was not ready to evaluate"));
	}
#endif // ENABLE_ANIM_DEBUG
   
	return bValid;
}
*/

void FAnimNode_IKHumanoidLegTrace::Initialize(const FAnimationInitializeContext& Context)
{
	const FBoneContainer& RequiredBones = Context.AnimInstanceProxy->GetRequiredBones();
	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize Humanoid IK Leg -- Leg invalid"));
#endif // ENABLE_IK_DEBUG

		return;
	}

	if (!Leg->InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize Humanoid IK Leg"));
#endif // ENABLE_IK_DEBUG
	}
}


