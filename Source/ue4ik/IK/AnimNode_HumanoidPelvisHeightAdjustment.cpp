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
		PelvisBone, MaxPelvisAdjustHeight, LeftTraceData, bEnableDebugDraw);

	FHumanoidIKTraceData RightTraceData;
	FHumanoidIK::HumanoidIKLegTrace(Character, Output.Pose, RightLeg,
		PelvisBone, MaxPelvisAdjustHeight, RightTraceData, bEnableDebugDraw);
	
	// Find the foot that's farthest from the ground. Transition the hips downward so it's the height
	// is where it would be, over flat ground.

	bool bReturnToCenter = false;
	float TargetPelvisDelta;

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
		FVector RootPosition = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, FCompactPoseBoneIndex(0));

		DebugDrawUtil::DrawSphere(World, SkelComp->ComponentToWorld.TransformPosition(RootPosition));

		FVector LeftFootCS  = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, LeftLeg.ShinBone.BoneIndex);
		FVector RightFootCS = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, RightLeg.ShinBone.BoneIndex);		

		float LeftTargetHeight  = LeftFootCS.Z - RootPosition.Z;
		float RightTargetHeight = RightFootCS.Z - RootPosition.Z;

		float LeftActualHeight  = LeftFootCS.Z - LeftFootFloorCS.Z;
		float RightActualHeight = RightFootCS.Z - RightFootFloorCS.Z; 

		float LeftDifference  = LeftActualHeight - LeftTargetHeight;
		float RightDifference = RightActualHeight - RightTargetHeight;

		if (LeftDifference < 0.0f && RightDifference < 0.0f)
		{
			bReturnToCenter = true;
		}
		else
		{
			TargetPelvisDelta = -1*FMath::Max(LeftDifference, RightDifference);
		}				
	}
	
	// Apply pelvis height adjustment
	if (bReturnToCenter)
	{
		TargetPelvisDelta = 0.0f;
	}
	
	FTransform PelvisTransformCS = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, PelvisBone.BoneIndex);	
	FVector PelvisLocationCS = PelvisTransformCS.GetLocation();
	PelvisLocationCS.Z += TargetPelvisDelta;
	PelvisTransformCS.SetLocation(PelvisLocationCS);

	OutBoneTransforms.Add(FBoneTransform(PelvisBone.BoneIndex, PelvisTransformCS));
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
	UE_LOG(LogIK, Warning, TEXT("Initializing biped leg IK..."));
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


