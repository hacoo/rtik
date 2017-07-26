// Copyright (c) Henry Cooney 2017

#include "AnimNode_HumanoidPelvisHeightAdjustment.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
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

void FAnimNode_HumanoidPelvisHeightAdjustment::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, 
	TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_HumanoidPelvisHeightAdjust_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

	if (LeftLeg == nullptr || RightLeg == nullptr || PelvisBone == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Could not evaluate Humanoid Pelvis Height Adjustment, a bone wrapper was null"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		return;
	}

	if (LeftLegTraceData == nullptr || RightLegTraceData == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Could not evaluate Humanoid Pelvis Height Adjustment, a trace data input was null"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		return;
	}
	

	USkeletalMeshComponent* SkelComp = Output.AnimInstanceProxy->GetSkelMeshComponent();
	ACharacter* Character = Cast<ACharacter>(SkelComp->GetOwner());
	if(Character == nullptr)
	{
		UE_LOG(LogIK, Warning, TEXT("FAnimNode_HumanoidPelvisHeightAdjustment -- evaluation failed, skeletal mesh component owner could not be cast to ACharacter"));
		return;
	}

	UWorld* World = Character->GetWorld();

	// Find the foot that's farthest from the ground. Transition the hips downward so it's the height
	// is where it would be, over flat ground.

	bool bReturnToCenter = false;
	float TargetPelvisDelta = 0.0f;

	if (LeftLegTraceData->GetTraceData().FootHitResult.GetActor()  == nullptr || 
		RightLegTraceData->GetTraceData().FootHitResult.GetActor() == nullptr) 
	{
		bReturnToCenter = true;
	}
	else	
	{
		// Check in component space; this way character rotation doesn't matter
		FVector ToCS = -1 * SkelComp->GetComponentLocation();
		FVector LeftFootFloorCS;
		FVector RightFootFloorCS;

		FVector LeftFootCS       = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, LeftLeg->Chain.ShinBone.BoneIndex);
		FVector RightFootCS      = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, RightLeg->Chain.ShinBone.BoneIndex);		
		FVector RootCS           = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, FCompactPoseBoneIndex(0));
		
		LeftLeg->Chain.GetIKFloorPointCS(*SkelComp, LeftLegTraceData->GetTraceData(), LeftFootFloorCS);
		RightLeg->Chain.GetIKFloorPointCS(*SkelComp, RightLegTraceData->GetTraceData(), RightFootFloorCS);	
		
/*		
		// The animroot, assumed to rest on the floor. The original animation assumed the floor was this high.
		// The adjusted animation should maintain a similar relationship to the (possibly uneven) floor.

		FVector LeftFootCS       = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, LeftLeg->Chain.ShinBone.BoneIndex);
		FVector RightFootCS      = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, RightLeg->Chain.ShinBone.BoneIndex);		

		float LeftTargetDelta    = LeftFootCS.Z - RootPosition.Z;
		float RightTargetDelta   = RightFootCS.Z - RootPosition.Z;

		float LeftTargetHeight   = LeftFootFloorCS.Z + LeftTargetDelta;
		float RightTargetHeight  = RightFootFloorCS.Z + RightTargetDelta;
		TargetPelvisDelta        = LeftTargetHeight < RightTargetHeight ?
			LeftTargetHeight - LeftFootCS.Z : RightTargetHeight - RightFootCS.Z; 
*/

		// Move so the lowest floor point is in reach
		TargetPelvisDelta = LeftFootFloorCS.Z < RightFootFloorCS.Z ?
			LeftFootFloorCS.Z - RootCS.Z : RightFootFloorCS.Z - RootCS.Z;

		if (FMath::Abs(TargetPelvisDelta) > MaxPelvisAdjustSize)
		{
			bReturnToCenter = true;
			TargetPelvisDelta = 0.0f;
		}
		
	}
   
	
	FVector TargetPelvisDeltaVec(0.0f, 0.0f, TargetPelvisDelta);
	FTransform PelvisTransformCS = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, PelvisBone->Bone.BoneIndex);	
	FVector PelvisTargetCS       = PelvisTransformCS.GetLocation() + TargetPelvisDeltaVec;

	FVector PreviousPelvisLoc    = PelvisTransformCS.GetLocation() + LastPelvisOffset;
	
	FVector PelvisAdjustVec      = LastPelvisOffset + (PelvisTargetCS - PreviousPelvisLoc).GetClampedToMaxSize(PelvisAdjustVelocity * DeltaTime);
	FVector NewPelvisLoc         = PelvisTransformCS.GetLocation() + PelvisAdjustVec;

	LastPelvisOffset             = PelvisAdjustVec;

	PelvisTransformCS.SetLocation(NewPelvisLoc);

	OutBoneTransforms.Add(FBoneTransform(PelvisBone->Bone.BoneIndex, PelvisTransformCS));

#if WITH_EDITOR
	if (bEnableDebugDraw)
	{
		FVector PelvisLocWorld = FAnimUtil::GetBoneWorldLocation(*SkelComp, Output.Pose, PelvisBone->Bone.BoneIndex);
		FTransform PelvisTarget(PelvisTransformCS);
		FAnimationRuntime::ConvertCSTransformToBoneSpace(SkelComp->GetComponentTransform(), Output.Pose,
			PelvisTarget, PelvisBone->Bone.BoneIndex, BCS_WorldSpace);
		
		FDebugDrawUtil::DrawSphere(World, PelvisLocWorld, FColor(255, 0, 0), 20.0f);

		if (bReturnToCenter)
		{
			FDebugDrawUtil::DrawSphere(World, PelvisTarget.GetLocation(), FColor(255, 255, 0), 20.0f);
			FDebugDrawUtil::DrawLine(World, PelvisLocWorld, PelvisTarget.GetLocation(), FColor(255, 255, 0));
			FVector TextOffset = FVector(0.0f, 0.0f, 100.0f);
			float AdjustHeight = (PelvisLocWorld - PelvisTarget.GetLocation()).Size();
			FString AdjustStr = FString::Printf(TEXT("Pelvis Offset: %f"), AdjustHeight);
			FDebugDrawUtil::DrawString(World, TextOffset, AdjustStr, Character, FColor(255, 255, 0));
		} 
		else
		{
			FDebugDrawUtil::DrawSphere(World, PelvisTarget.GetLocation(), FColor(0, 0, 255), 20.0f);
			FDebugDrawUtil::DrawLine(World, PelvisLocWorld, PelvisTarget.GetLocation(), FColor(0, 0, 255));
			FVector TextOffset = FVector(0.0f, 0.0f, 100.0f);
			float AdjustHeight = (PelvisLocWorld - PelvisTarget.GetLocation()).Size();
			FString AdjustStr = FString::Printf(TEXT("Pelvis Offset: %f"), AdjustHeight);
			FDebugDrawUtil::DrawString(World, TextOffset, AdjustStr, Character, FColor(0, 0, 255));
		}		

		FVector LeftTraceWorld = LeftLegTraceData->GetTraceData().FootHitResult.ImpactPoint; 
		FDebugDrawUtil::DrawSphere(World, LeftTraceWorld, FColor(0, 255, 0), 20.0f); 

		FVector RightTraceWorld = RightLegTraceData->GetTraceData().FootHitResult.ImpactPoint; 
		FDebugDrawUtil::DrawSphere(World, RightTraceWorld, FColor(255, 0, 0), 20.0f); 

	}
#endif // WITH_EDITOR
}

bool FAnimNode_HumanoidPelvisHeightAdjustment::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	
	if (LeftLeg == nullptr || RightLeg == nullptr || PelvisBone == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid Pelvis Height Adjustment was not valid -- one of the bone wrappers was null"));				
#endif // ENABLE_ANIM_DEBUG
		return false;
	}

	bool bValid = LeftLeg->InitIfInvalid(RequiredBones)
		&& RightLeg->InitIfInvalid(RequiredBones)
		&& PelvisBone->InitIfInvalid(RequiredBones);

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

	if (LeftLeg == nullptr || RightLeg == nullptr || PelvisBone == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize biped hip adjustment -- one of the bone wrappers was null"));
#endif // ENABLE_IK_DEBUG
		return;
	}

	if (!RightLeg->InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize right leg for biped hip adjustment"));
#endif // ENABLE_IK_DEBUG
	}

	if (!LeftLeg->InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize left leg for biped hip adjustment"));
#endif // ENABLE_IK_DEBUG
	}

	if (!PelvisBone->Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize pelvis bone for biped hip adjustment"));
#endif // ENABLE_IK_DEBUG
	}	
}


