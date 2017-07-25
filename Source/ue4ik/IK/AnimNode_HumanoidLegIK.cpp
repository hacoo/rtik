// Copyright(c) Henry Cooney 2017

#include "AnimNode_HumanoidLegIK.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "Utility/AnimUtil.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Leg IK Eval"), STAT_HumanoidLegIK_Eval, STATGROUP_Anim);

void FAnimNode_HumanoidLegIK::Initialize(const FAnimationInitializeContext & Context)
{
	Super::Initialize(Context);
	BaseComponentPose.Initialize(Context);
}

void FAnimNode_HumanoidLegIK::CacheBones(const FAnimationCacheBonesContext & Context)
{
	Super::CacheBones(Context);
	BaseComponentPose.CacheBones(Context);
}

void FAnimNode_HumanoidLegIK::UpdateInternal(const FAnimationUpdateContext & Context)
{
	BaseComponentPose.Update(Context);
	DeltaTime = Context.GetDeltaTime();	
}

void FAnimNode_HumanoidLegIK::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_HumanoidLegIK_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

	// Input pin pointers are checked in IsValid -- don't need to check here

	USkeletalMeshComponent* SkelComp   = Output.AnimInstanceProxy->GetSkelMeshComponent();
	
	FMatrix ToCS               = SkelComp->ComponentToWorld.ToMatrixNoScale().Inverse();
	FVector HipCS              = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.HipBone.BoneIndex);
	FVector KneeCS             = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.ThighBone.BoneIndex);
	FTransform FootCSTransform = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, Leg->Chain.ShinBone.BoneIndex);
	FVector FootCS             = FootCSTransform.GetLocation();
	FVector FloorCS            = ToCS.TransformPosition(TraceData->GetTraceData().FootHitResult.ImpactPoint);

	FVector FootTargetCS;
	
	if (Mode == EHumanoidLegIKMode::IK_Human_Leg_Locomotion)
	{
		// Use trace data to figure out where the foot should go.
		FComponentSpacePoseContext BasePose(Output);
		BaseComponentPose.EvaluateComponentSpace(BasePose);

		FVector BaseRootCS = FAnimUtil::GetBoneCSLocation(*SkelComp, BasePose.Pose, FCompactPoseBoneIndex(0));
		FVector BaseFootCS = FAnimUtil::GetBoneCSLocation(*SkelComp, BasePose.Pose, Leg->Chain.FootBone.BoneIndex);

		// If foot is X cm above animroot before adjustment, it should be X cm above the floor trace impact after adjustment
		float HeightAboveRoot = BaseFootCS.Z - BaseRootCS.Z;
		float MinimumHeight = FloorCS.Z + HeightAboveRoot + Leg->Chain.FootRadius;

		// Don't apply IK unless the foot is below the target height
		if (FootCS.Z < MinimumHeight)
		{
			FootTargetCS = FVector(FootCS.X, FootCS.Y, FloorCS.Z + HeightAboveRoot + Leg->Chain.FootRadius);
		}
		else
		{
			FootTargetCS = FootCS;
		}
	}
	else
	{
		FootTargetCS = ToCS.TransformPosition(FootTargetWorld);
	}

	// Interpolate the foot target (if needed)
	if (bEffectorMovesInstantly)
	{
		LastEffectorOffset = FVector(0.0f, 0.0f, 0.0f);		
	}
	else
	{
		FVector CurrentFootPosition = FootCS + LastEffectorOffset;
		FVector RequiredDelta       = FootTargetCS - CurrentFootPosition;
		float DeltaSq               = RequiredDelta.SizeSquared();

		if (DeltaSq > (EffectorVelocity * EffectorVelocity * DeltaTime))
		{
			RequiredDelta = RequiredDelta.GetClampedToMaxSize(EffectorVelocity * DeltaTime);			
		}

		FootTargetCS                = CurrentFootPosition + RequiredDelta;
		LastEffectorOffset          = LastEffectorOffset + RequiredDelta;
	}

	if (Mode != EHumanoidLegIKMode::IK_Human_Leg_Locomotion)
	{
		// Ensure that the target is reachable; if not do not apply IK
		// This only tests distance to the target. Future work: check ROM as well
		// No need to check if in locomotion mode; target should always be reachable.

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
	FabrikSolver.EffectorRotationSource = EffectorRotationSource;
	FTransform EffectorTransform(FootCSTransform);
	EffectorTransform.SetLocation(FootTargetCS);
	FabrikSolver.EffectorTransform = EffectorTransform;

	FTransform FootLocalTransform = Output.Pose.GetLocalSpaceTransform(Leg->Chain.FootBone.BoneIndex);

	// Internal fabrik solver will fill in OutBoneTransforms. Stock solver does not handle ROMs
	FabrikSolver.EvaluateSkeletalControl_AnyThread(Output, OutBoneTransforms);
	
#if WITH_EDITOR
	if (bEnableDebugDraw)
	{
		UWorld* World = SkelComp->GetWorld();		
		FMatrix ToWorld = SkelComp->ComponentToWorld.ToMatrixNoScale();
		FVector EffectorWorld = ToWorld.TransformPosition(FootTargetCS);
		FDebugDrawUtil::DrawSphere(World, EffectorWorld, FColor(255, 0, 255));
		FDebugDrawUtil::DrawSphere(World, ToWorld.TransformPosition(FloorCS), FColor(255, 0, 0));
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
