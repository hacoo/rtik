// Copyright(c) Henry Cooney 2017

#include "AnimNode_HumanoidLegIK.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Runtime/AnimationCore/Public/TwoBoneIK.h"
#include "Utility/AnimUtil.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Leg IK Eval"), STAT_HumanoidLegIK_Eval, STATGROUP_Anim);

void FAnimNode_HumanoidLegIK::UpdateInternal(const FAnimationUpdateContext & Context)
{
	DeltaTime = Context.GetDeltaTime();
}

void FAnimNode_HumanoidLegIK::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_HumanoidLegIK_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

/*
	if (Leg == nullptr || TraceData == nullptr)
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
*/
	
	USkeletalMeshComponent* SkelComp = Output.AnimInstanceProxy->GetSkelMeshComponent();
	
	FVector HipCS     = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.HipBone.BoneIndex);
	FVector KneeCS    = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.ThighBone.BoneIndex);
	FVector FootCS    = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.ShinBone.BoneIndex);
<<<<<<< HEAD

	FMatrix ToCS      = SkelComp->ComponentToWorld.ToMatrixNoScale().Inverse();

	FVector FootTargetCS;
=======
	 
	// Reachability is checked by max extension length only -- not ROM
	FVector HipToTarget = (FootTargetCS - HipCS);
	float LegLengthSq = FMath::Square(Leg->Chain.GetTotalChainLength());
	float HipToTargetLengthSq = HipToTarget.SizeSquared();
>>>>>>> 752aa92b1a43c97a593516ef6a35254aa1142116

	if (Mode == EHumanoidLegIKMode::IK_Human_Leg_WorldLocation)
	{
		FootTargetCS = ToCS.TransformPosition(FootTargetWorld);
	}
	else
	{
		// Use trace data to figure out where the foot should go.
		// If foot is X cm above animroot before adjustment, it should be X cm above the floor trace impact after adjustment
		
		FVector RootCS = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, FCompactPoseBoneIndex(0));
		FVector FloorCS = ToCS.TransformPosition(TraceData->GetTraceData().FootHitResult.ImpactPoint);
		float HeightAboveRoot = FootCS.Z - RootCS.Z;
		FootTargetCS = FVector(FootCS.X, FootCS.Y, FloorCS.Z + HeightAboveRoot);
	}

	if (Mode != EHumanoidLegIKMode::IK_Human_Leg_Locomotion)
	{
		// Ensure that the target is reachable; if not do not apply IK
		// This only tests distance to the target. Future work: check ROM as well
		// No need to check if in locomotion mode; target shoudl always be reachable.

		FVector HipToTarget = (FootTargetCS - HipCS);
		float LegLengthSq = FMath::Square(Leg->Chain.GetTotalChainLength());
		float HipToTargetLengthSq = HipToTarget.SizeSquared();

		if (HipToTargetLengthSq > LegLengthSq)
		{
			switch (UnreachableRule)
			{
			case EIKUnreachableRule::IK_Abort:
				return;
			case EIKUnreachableRule::IK_DragRoot:
#if ENABLE_IK_DEBUG
				UE_LOG(LogIK, Warning, TEXT("Humanoid Leg IK Solver does not support IK Unreachable Rule Drag Root"));
#endif // ENABLE_IK_DEBUG			
				return;
			case EIKUnreachableRule::IK_Reach:
				break;
			}
		}
	}
	
	FabrikSolver.EffectorTransformSpace = EBoneControlSpace::BCS_ComponentSpace;
	FabrikSolver.EffectorTransform = FTransform(FootTargetCS);

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
	if (Leg == nullptr || TraceData == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid IK Leg was not valid to evaluate -- an input wrapper object was null"));		
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

	if (Leg == nullptr || TraceData == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize Humanoid IK Leg -- An input wrapper object was null"));
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


