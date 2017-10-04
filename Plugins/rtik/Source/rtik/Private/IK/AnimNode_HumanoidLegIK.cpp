// Copyright(c) Henry Cooney 2017

#include "rtik.h"
#include "AnimNode_HumanoidLegIK.h"
#include "Components/SkeletalMeshComponent.h"
#include "TwoBoneIK.h"
#include "RangeLimitedFABRIK.h"
#include "Utility/AnimUtil.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Leg IK Eval"), STAT_HumanoidLegIK_Eval, STATGROUP_Anim);

void FAnimNode_HumanoidLegIK::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{
	Super::Initialize_AnyThread(Context);
	BaseComponentPose.Initialize(Context);
}

void FAnimNode_HumanoidLegIK::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{
	Super::CacheBones_AnyThread(Context);
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
	
	FMatrix ToCS               = SkelComp->GetComponentToWorld().ToMatrixNoScale().Inverse();
	FTransform HipCSTransform  = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, Leg->Chain.HipBone.BoneIndex);
	FTransform KneeCSTransform = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, Leg->Chain.ThighBone.BoneIndex);
	FTransform FootCSTransform = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, Leg->Chain.ShinBone.BoneIndex);
	FVector HipCS              = HipCSTransform.GetLocation();
	FVector KneeCS             = KneeCSTransform.GetLocation();
	FVector FootCS             = FootCSTransform.GetLocation();

	FVector FootTargetCS;
	FVector FloorCS;
			
	if (Mode == EHumanoidLegIKMode::IK_Human_Leg_Locomotion)
	{		
		// Check that we have some valid trace data
		if (TraceData->GetTraceData().FootHitResult.GetActor() == nullptr &&
			TraceData->GetTraceData().ToeHitResult.GetActor() == nullptr)
		{
#if ENABLE_IK_DEBUG_VERBOSE
			UE_LOG(LogRTIK, Warning, TEXT("Leg IK trace did not hit a valid actor"));
#endif
			return;
		}

		// Use trace data to figure out where the foot should go.
		FComponentSpacePoseContext BasePose(Output);
		BaseComponentPose.EvaluateComponentSpace(BasePose);

		// If within foot rotation limit, use the low point. Otherwise, use the higher point and the foot shouldn't rotate.
		bool bWithinRotationLimit = Leg->Chain.GetIKFloorPointCS(*SkelComp, TraceData->GetTraceData(), FloorCS);

		FVector BaseRootCS = FAnimUtil::GetBoneCSLocation(*SkelComp, BasePose.Pose, FCompactPoseBoneIndex(0));
		FVector BaseFootCS = FAnimUtil::GetBoneCSLocation(*SkelComp, BasePose.Pose, Leg->Chain.ShinBone.BoneIndex);
		
		// How high the foot should be above the root. If below this, IK turns on.
		float FootHeightAboveRoot = BaseFootCS.Z - BaseRootCS.Z;

		// Old method included FootRadius -- could cause IK to cut in suddenly during level movement. Leaving in for historical interest
		// float MinimumHeight = FloorCS.Z + HeightAboveRoot + Leg->Chain.FootRadius;		

		float MinimumFootHeight = FloorCS.Z + FootHeightAboveRoot;

		// Don't move foot unless the foot is below the target height
		if (FootCS.Z < MinimumFootHeight)
		{
			//Again, foot radius is now ignored. Don't need it 
			//FootTargetCS = FVector(FootCS.X, FootCS.Y, FloorCS.Z + FootHeightAboveRoot + Leg->Chain.FootRadius);
			FootTargetCS = FVector(FootCS.X, FootCS.Y, FloorCS.Z + FootHeightAboveRoot);
		}
		else
		{
			FootTargetCS = FootCS;
		}

	}
	else
	{
		FootTargetCS = ToCS.TransformPosition(FootTargetWorld.GetLocation());
	}

	// Interpolate the foot target (if needed)
	if (bEffectorMovesInstantly)
	{
		LastEffectorOffset = FVector(0.0f, 0.0f, 0.0f);		
	}
	else
	{
		FVector OffsetFootPos = FootCS + LastEffectorOffset;
		FVector RequiredDelta = FootTargetCS - OffsetFootPos;

		if (RequiredDelta.Size() > EffectorVelocity * DeltaTime)
		{
			RequiredDelta = RequiredDelta.GetClampedToMaxSize(EffectorVelocity * DeltaTime);
		}
		
		FootTargetCS          = OffsetFootPos + RequiredDelta;
		LastEffectorOffset    = LastEffectorOffset + RequiredDelta;
	}

	// Will contain post-IK transforms	
	TArray<FTransform> DestCSTransforms;

	if (Solver == EHumanoidLegIKSolver::IK_Human_Leg_Solver_FABRIK)
	{
		// Gather bone transforms and constraints
		TArray<FTransform> SourceCSTransforms({
			HipCSTransform,
			KneeCSTransform,
			FootCSTransform
		});

		TArray<FIKBoneConstraint*> Constraints({
			Leg->Chain.HipBone.GetConstraint(),
			Leg->Chain.ThighBone.GetConstraint(),
			Leg->Chain.ShinBone.GetConstraint()
		});

		bool bBoneLocationUpdated = FRangeLimitedFABRIK::SolveRangeLimitedFABRIK(
			SourceCSTransforms,
			Constraints,
			FootTargetCS,
			DestCSTransforms,
			0.0f,
			1.0f,
			Precision,
			MaxIterations,
			Cast<ACharacter>(SkelComp->GetOwner())
		);
	}
	else if (Solver == EHumanoidLegIKSolver::IK_Human_Leg_Solver_TwoBone)
	{
		DestCSTransforms.Reserve(3);
		DestCSTransforms.Add(HipCSTransform);
		DestCSTransforms.Add(KneeCSTransform);
		DestCSTransforms.Add(FootCSTransform);

		AnimationCore::SolveTwoBoneIK(
			DestCSTransforms[0],
			DestCSTransforms[1],
			DestCSTransforms[2],
			FVector(1.0f, 0.0f, 0.0f), // Use forward direction, knee correction node will fix it
			FootTargetCS,
			false,
			1.0f,
			1.0f
		);
	}

	OutBoneTransforms.Add(FBoneTransform(Leg->Chain.HipBone.BoneIndex, DestCSTransforms[0]));
	OutBoneTransforms.Add(FBoneTransform(Leg->Chain.ThighBone.BoneIndex, DestCSTransforms[1]));
	OutBoneTransforms.Add(FBoneTransform(Leg->Chain.ShinBone.BoneIndex, DestCSTransforms[2]));

#if WITH_EDITOR
	if (bEnableDebugDraw)
	{
		UWorld* World = SkelComp->GetWorld();		
		FMatrix ToWorld = SkelComp->GetComponentToWorld().ToMatrixNoScale();
		FVector EffectorWorld = ToWorld.TransformPosition(FootTargetCS);

		FDebugDrawUtil::DrawSphere(World, EffectorWorld, FColor(255, 0, 255));
		FDebugDrawUtil::DrawSphere(World, ToWorld.TransformPosition(FloorCS), FColor(255, 0, 0));
		FDebugDrawUtil::DrawSphere(World, TraceData->GetTraceData().FootHitResult.ImpactPoint, FColor(255, 255, 0), 10.0f);
		FDebugDrawUtil::DrawSphere(World, TraceData->GetTraceData().ToeHitResult.ImpactPoint, FColor(255, 255, 0), 10.0f);

		// Leg before IK, in yellow:
		FDebugDrawUtil::DrawLine(World,
			ToWorld.TransformPosition(HipCSTransform.GetLocation()),
			ToWorld.TransformPosition(KneeCSTransform.GetLocation()),
			FColor(255, 255, 0)
		);
		FDebugDrawUtil::DrawLine(World,
			ToWorld.TransformPosition(KneeCSTransform.GetLocation()),
			ToWorld.TransformPosition(FootCSTransform.GetLocation()),
			FColor(255, 255, 0)
		);

		// Leg after IK, in cyan
		FDebugDrawUtil::DrawLine(World,
			ToWorld.TransformPosition(DestCSTransforms[0].GetLocation()),
			ToWorld.TransformPosition(DestCSTransforms[1].GetLocation()),
			FColor(0, 255, 255)
		);
		FDebugDrawUtil::DrawLine(World,
			ToWorld.TransformPosition(DestCSTransforms[1].GetLocation()),
			ToWorld.TransformPosition(DestCSTransforms[2].GetLocation()),
			FColor(0, 255, 255)
		);	   
	}
#endif // WITH_EDITOR
}

bool FAnimNode_HumanoidLegIK::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	if (Leg == nullptr || TraceData == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogRTIK, Warning, TEXT("IK Node Humanoid IK Leg was not valid to evaluate -- an input wrapper object was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;
	}
	
	bool bValid = Leg->InitIfInvalid(RequiredBones);

#if ENABLE_IK_DEBUG_VERBOSE
	if (!bValid)
	{
		UE_LOG(LogRTIK, Warning, TEXT("IK Node Humanoid IK Leg was not valid to evaluate"));
	}
#endif // ENABLE_ANIM_DEBUG

	return bValid;
}

void FAnimNode_HumanoidLegIK::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{

	if (Leg == nullptr || TraceData == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialize Humanoid IK Leg -- An input wrapper object was null"));
#endif // ENABLE_IK_DEBUG

		return;
	}

	if (!Leg->InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialize Humanoid IK Leg"));
#endif // ENABLE_IK_DEBUG
	}
}
