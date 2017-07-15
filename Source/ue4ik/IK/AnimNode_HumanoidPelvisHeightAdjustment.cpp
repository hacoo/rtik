// Copyright (c) Henry Cooney 2017

#include "AnimNode_HumanoidPelvisHeightAdjustment.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Utility/AnimUtil.h"
#include "IK/IK.h"
#include "HumanoidIK.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

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
	if(Character == nullptr)
	{
		UE_LOG(LogIK, Warning, TEXT("FAnimNode_HumanoidPelvisHeightAdjustment -- evaluation failed, skeletal mesh component owner could not be cast to ACharacter"));
		return;
	}

	UWorld* World = Character->GetWorld();

	FHumanoidIKTraceData LeftTraceData;
	FHumanoidIK::HumanoidIKLegTrace(Character, Output.Pose, LeftLeg,
		PelvisBone, MaxPelvisAdjustSize, LeftTraceData, false);

	FHumanoidIKTraceData RightTraceData;
	FHumanoidIK::HumanoidIKLegTrace(Character, Output.Pose, RightLeg,
		PelvisBone, MaxPelvisAdjustSize, RightTraceData, false);
	
	// Find the foot that's farthest from the ground. Transition the hips downward so it's the height
	// is where it would be, over flat ground.

	bool bReturnToCenter = false;
	float TargetPelvisDelta = 0.0f;

	if (LeftTraceData.FootHitResult.GetActor()     == nullptr
		|| RightTraceData.FootHitResult.GetActor() == nullptr) 
	{
		bReturnToCenter = true;
	}
	else	
	{
		// Check in component space; this way charcter rotation doens't matter
		FMatrix ToCS             = SkelComp->ComponentToWorld.ToMatrixWithScale().Inverse();
		FVector LeftFootFloor    = LeftTraceData.FootHitResult.ImpactPoint;
		FVector RightFootFloor   = RightTraceData.FootHitResult.ImpactPoint;
		FVector LeftFootFloorCS  = ToCS.TransformPosition(LeftFootFloor);
		FVector RightFootFloorCS = ToCS.TransformPosition(RightFootFloor);
		
		// The animroot, assumed to rest on the floor. The original animation assumed the floor was this high.
		FVector RootPosition     = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, FCompactPoseBoneIndex(0));

		FVector LeftFootCS       = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, LeftLeg.ShinBone.BoneIndex);
		FVector RightFootCS      = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, RightLeg.ShinBone.BoneIndex);		

		float LeftTargetDelta    = LeftFootCS.Z - RootPosition.Z;
		float RightTargetDelta   = RightFootCS.Z - RootPosition.Z;

		float LeftTargetHeight   = LeftFootFloorCS.Z + LeftTargetDelta;
		float RightTargetHeight  = RightFootFloorCS.Z + RightTargetDelta;

		TargetPelvisDelta        = LeftTargetHeight < RightTargetHeight ?
			LeftTargetHeight - LeftFootCS.Z : RightTargetHeight - RightFootCS.Z; 

		if (TargetPelvisDelta > MaxPelvisAdjustSize)
		{
			bReturnToCenter = true;
			TargetPelvisDelta = 0.0f;
		}
		
	}
   
	FVector TargetPelvisDeltaVec(0.0f, 0.0f, TargetPelvisDelta);
	
	FTransform PelvisTransformCS = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, PelvisBone.BoneIndex);	
	FVector PelvisTargetCS       = PelvisTransformCS.GetLocation();
	PelvisTargetCS += TargetPelvisDeltaVec;

	FVector PreviousPelvisLoc    = PelvisTransformCS.GetLocation() + LastPelvisOffset;
	FVector PelvisAdjustVec      = (PelvisTargetCS - PreviousPelvisLoc).GetClampedToMaxSize(PelvisAdjustVelocity * DeltaTime);
	FVector NewPelvisLoc         = PelvisTransformCS.GetLocation() + PelvisAdjustVec;
	LastPelvisOffset             = PelvisAdjustVec;

	PelvisTransformCS.SetLocation(NewPelvisLoc);

	OutBoneTransforms.Add(FBoneTransform(PelvisBone.BoneIndex, PelvisTransformCS));

#if WITH_EDITOR
	if (bEnableDebugDraw)
	{
		FVector PelvisLocWorld = FAnimUtil::GetBoneWorldLocation(*SkelComp, Output.Pose, PelvisBone.BoneIndex);
		FVector PelvisTarget = PelvisLocWorld + TargetPelvisDeltaVec;
		
		FDebugDrawUtil::DrawSphere(World, PelvisLocWorld, FColor(0, 255, 255), 20.0f);

		if (bReturnToCenter)
		{
			FDebugDrawUtil::DrawSphere(World, PelvisTarget, FColor(255, 255, 0), 20.0f);
			//DebugDrawUtil::DrawLine(World, PelvisLocWorld, PelvisTarget, FColor(255, 255, 0));
		} 
		else
		{
			FDebugDrawUtil::DrawSphere(World, PelvisTarget, FColor(0, 0, 255), 20.0f);
			//DebugDrawUtil::DrawLine(World, PelvisLocWorld, PelvisTarget, FColor(0, 0, 255));
		}
	}
#endif // WITH_EDITOR
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


