// Copyright(c) Henry Cooney 2017

#include "AnimNode_HumanoidFootRotationController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "Utility/AnimUtil.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Foot Rotation Controller  Eval"), STAT_HumanoidFootRotationController_Eval, STATGROUP_Anim);

void FAnimNode_HumanoidFootRotationController::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_HumanoidFootRotationController_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

	// Input pin pointers are checked in IsValid -- don't need to check here
	USkeletalMeshComponent* SkelComp   = Output.AnimInstanceProxy->GetSkelMeshComponent();

	float RequiredRad = 0.0f;
	bool bTargetRotationWithinLimit = Leg->Chain.FindWithinFootRotationLimit(*SkelComp, TraceData->GetTraceData(), RequiredRad);

	FQuat TargetOffset = FQuat::Identity;		

	if (bTargetRotationWithinLimit)
	{
		// Compute required rotation
		FMatrix ToCS = SkelComp->GetComponentToWorld().ToMatrixNoScale().Inverse();
		
		FVector FootFloor     = TraceData->GetTraceData().FootHitResult.ImpactPoint;
		FVector ToeFloor      = TraceData->GetTraceData().ToeHitResult.ImpactPoint;
		FVector FloorSlopeVec = ToCS.TransformVector(ToeFloor - FootFloor);

		FVector FloorFlatVec(FloorSlopeVec);
		FloorFlatVec.Z = 0.0f;

		FVector KneeCS  = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.ThighBone.BoneIndex);
		FVector FootCS  = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.ShinBone.BoneIndex);
		FVector ToeCS   = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.FootBone.BoneIndex);

		FVector ShinVec = KneeCS - FootCS;
		FVector FootVec = ToeCS - FootCS;

		FVector RotationAxis = FVector::CrossProduct(FootVec, ShinVec);

		if (RotationAxis.Normalize())
		{
			if (FVector::DotProduct(RotationAxis, FVector::CrossProduct(FloorFlatVec, FloorSlopeVec).GetUnsafeNormal()) < 0.0f)
			{
				RotationAxis *= -1.0f;
			}
		   
			TargetOffset = FQuat(RotationAxis, RequiredRad);
		}
	}

	// Interpolate to target rotation and apply 
	LastRotationOffset = FQuat::Slerp(LastRotationOffset, TargetOffset, FMath::Clamp(RotationSlerpSpeed * DeltaTime, 0.0f, 1.0f));

	FTransform FootCSTransform = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, Leg->Chain.ShinBone.BoneIndex);
	FQuat NewRotation          = LastRotationOffset * FootCSTransform.GetRotation();

	FootCSTransform.SetRotation(NewRotation);
   
	OutBoneTransforms.Add(FBoneTransform(Leg->Chain.ShinBone.BoneIndex, FootCSTransform));
   	
#if WITH_EDITOR
	if (bEnableDebugDraw)
	{
		UWorld* World = SkelComp->GetWorld();
		ACharacter* Character = Cast<ACharacter>(SkelComp->GetOwner());
		FMatrix ToWorld = SkelComp->GetComponentToWorld().ToMatrixNoScale();
		if (bTargetRotationWithinLimit)
		{
			FDebugDrawUtil::DrawLine(World,
				TraceData->GetTraceData().FootHitResult.ImpactPoint,
				TraceData->GetTraceData().ToeHitResult.ImpactPoint,
				FColor(0, 255, 0));

			FVector TextOffset(0.0f, 0.0f, 100.0f);
			FString AngleStr = FString::Printf(TEXT("Floor angle (deg): %f"), FMath::RadiansToDegrees(RequiredRad));
			FDebugDrawUtil::DrawString(World, TextOffset, AngleStr, Character, FColor(0, 255, 0));
		}
		else
		{
			FDebugDrawUtil::DrawLine(World,
				TraceData->GetTraceData().FootHitResult.ImpactPoint,
				TraceData->GetTraceData().ToeHitResult.ImpactPoint,
				FColor(255, 0, 0));

			FVector TextOffset(0.0f, 0.0f, 100.0f);
			FString AngleStr = FString::Printf(TEXT("Floor angle (deg): %f"), FMath::RadiansToDegrees(RequiredRad));
			FDebugDrawUtil::DrawString(World, TextOffset, AngleStr, Character, FColor(255, 0, 0));
		}
	}
#endif // WITH_EDITOR
}

bool FAnimNode_HumanoidFootRotationController::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	if (Leg == nullptr || TraceData == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Humanoid Foot Rotation Controller was not valid to evaluate -- an input wrapper object was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;
	}
	
	bool bValid = Leg->InitIfInvalid(RequiredBones);

#if ENABLE_IK_DEBUG_VERBOSE
	if (!bValid)
	{
		UE_LOG(LogIK, Warning, TEXT("Humanoid Foot Rotation Controller was not valid to evaluate"));
	}
#endif // ENABLE_ANIM_DEBUG

	return bValid;
}

void FAnimNode_HumanoidFootRotationController::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{

	if (Leg == nullptr || TraceData == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize Humanoid Foot Rotation Controller-- An input wrapper object was null"));
#endif // ENABLE_IK_DEBUG

		return;
	}

	if (!Leg->InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize Humanoid Foot Rotation Controller"));
#endif // ENABLE_IK_DEBUG
	}
}