// Copyright(c) Henry Cooney 2017

#include "AnimNode_HumanoidLegIK.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Runtime/AnimationCore/Public/TwoBoneIK.h"
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

	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Could not evaluate Humanoid Leg IK, Leg was null"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		return;
	}

	if (!bEnable)
	{
		return;
	}
	
	USkeletalMeshComponent* SkelComp = Output.AnimInstanceProxy->GetSkelMeshComponent();

	FMatrix ToCS         = SkelComp->ComponentToWorld.ToMatrixNoScale().Inverse();
	FVector FootTargetCS = ToCS.TransformPosition(FootTargetWorld);
	
	FVector HipCS     = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.HipBone.BoneIndex);
	FVector KneeCS    = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.ThighBone.BoneIndex);
	FVector FootCS    = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.ShinBone.BoneIndex);
	 
	// Ensure that the target is reachable; if not do not apply IK
    // This only tests distance to the target. Future work: check ROM as well

	FVector HipToTarget = (FootTargetCS - HipCS);
	float LegLengthSq = FMath::Square(Leg->Chain.GetTotalChainLength());
	float HipToTargetLengthSq = HipToTarget.SizeSquared();

	if (HipToTargetLengthSq > LegLengthSq)
	{
		switch (UnreachableRule)
		{
		case EIKUnreachableRule::IK_Abort : 
			return;
		case EIKUnreachableRule::IK_DragRoot : 
#if ENABLE_IK_DEBUG
			UE_LOG(LogIK, Warning, TEXT("Humanoid Leg IK Solver does not support IK Unreachable Rule Drag Root"));
#endif // ENABLE_IK_DEBUG			
			return;
		case EIKUnreachableRule::IK_Reach : 
			break;
		}
	}
	
	FTransform EffectorTransform(FootTargetCS);	

	FabrikSolver.EffectorTransformSpace = EBoneControlSpace::BCS_ComponentSpace;
	FabrikSolver.EffectorTransform = EffectorTransform;

	// Internal fabrik solver will fill in OutBoneTransforms. Stock solver does not handle ROMs
	FabrikSolver.EvaluateSkeletalControl_AnyThread(Output, OutBoneTransforms);
   
#if WITH_EDITOR
	if (bEnableDebugDraw)
	{
		UWorld* World = SkelComp->GetWorld();
		FDebugDrawUtil::DrawSphere(World, FootTargetWorld, FColor(0, 255, 255));
	}
#endif // WITH_EDITOR
}

bool FAnimNode_HumanoidLegIK::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
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

void FAnimNode_HumanoidLegIK::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{

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
	else
	{
		// Set up FABRIK solver
		FabrikSolver.ActualAlpha = 1.0f;
		FabrikSolver.TipBone = Leg->Chain.ShinBone.BoneRef;
		FabrikSolver.RootBone = Leg->Chain.HipBone.BoneRef;
		FabrikSolver.Precision = Precision;
		FabrikSolver.MaxIterations = MaxIterations;
	}
}


