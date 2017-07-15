// Copyright(c) Henry Cooney 2017

#include "AnimNode_HumanoidLegIK.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Utility/AnimUtil.h"
#include "IK/IK.h"
#include "HumanoidIK.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Pelvis Height Adjust Eval"), STAT_HumanoidPelvisHeightAdjust_Eval, STATGROUP_Anim);

void FAnimNode_HumanoidLegIK::UpdateInternal(const FAnimationUpdateContext & Context)
{
	DeltaTime = Context.GetDeltaTime();
}

void FAnimNode_HumanoidLegIK::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_HumanoidPelvisHeightAdjust_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

}

bool FAnimNode_HumanoidLegIK::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	bool bValid = true;

#if ENABLE_IK_DEBUG_VERBOSE
	if (!bValid)
	{
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid IK Leg was not valid to evaluate"));
	}
#endif // ENABLE_ANIM_DEBUG

	return bValid;
}

void FAnimNode_HumanoidLegIK::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	
}


