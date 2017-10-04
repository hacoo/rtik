// Copyright(c) Henry Cooney 2017

#include "rtik.h"
#include "AnimNode_HumanoidLegIKKneeCorrection.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "Utility/AnimUtil.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Knee Correction Eval"), STAT_HumanoidLegIKKneeCorrection_Eval, STATGROUP_Anim);

void FAnimNode_HumanoidLegIKKneeCorrection::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{
	Super::Initialize_AnyThread(Context);
	BaseComponentPose.Initialize(Context);
}

void FAnimNode_HumanoidLegIKKneeCorrection::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{
	Super::CacheBones_AnyThread(Context);
	BaseComponentPose.CacheBones(Context);
}

void FAnimNode_HumanoidLegIKKneeCorrection::UpdateInternal(const FAnimationUpdateContext & Context)
{
	BaseComponentPose.Update(Context);
	DeltaTime = Context.GetDeltaTime();	
}

void FAnimNode_HumanoidLegIKKneeCorrection::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_HumanoidLegIKKneeCorrection_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

	USkeletalMeshComponent* SkelComp = Output.AnimInstanceProxy->GetSkelMeshComponent();

	FComponentSpacePoseContext BasePose(Output);
	BaseComponentPose.EvaluateComponentSpace(BasePose);

	// Pre-IK positions
	FVector HipCSPre      = FAnimUtil::GetBoneCSLocation(*SkelComp, BasePose.Pose, Leg->Chain.HipBone.BoneIndex);
	FVector KneeCSPre     = FAnimUtil::GetBoneCSLocation(*SkelComp, BasePose.Pose, Leg->Chain.ThighBone.BoneIndex);
	FVector FootCSPre     = FAnimUtil::GetBoneCSLocation(*SkelComp, BasePose.Pose, Leg->Chain.ShinBone.BoneIndex);
	FVector ToeCSPre      = FAnimUtil::GetBoneCSLocation(*SkelComp, BasePose.Pose, Leg->Chain.FootBone.BoneIndex);

	// Post-IK positions
	FVector HipCSPost     = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.HipBone.BoneIndex);
	FVector KneeCSPost    = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.ThighBone.BoneIndex);
	FVector FootCSPost    = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.ShinBone.BoneIndex);
	FVector ToeCSPost     = FAnimUtil::GetBoneCSLocation(*SkelComp, Output.Pose, Leg->Chain.FootBone.BoneIndex);

	// Thigh and shin before correction
	FVector OldThighVec = (KneeCSPost - HipCSPost).GetUnsafeNormal();
	FVector OldShinVec  = (FootCSPost - KneeCSPost).GetUnsafeNormal();
	
	// If the leg is fully extended or fully folded, early out (correction is never needed)
	if (FMath::IsNearlyEqual(FMath::Abs(FVector::DotProduct(OldThighVec, OldShinVec)), 1.0f))
	{
		return;
	}
  
	// To correct the knee:
	// - Project everything onto the plane normal to the vector from thigh to foot
	// - Find the angle, in the base pose, between the direction of the foot and the direction of the knee. If the leg is fully extended, assume this angle is 0.	
	// - Rotate the IKed knee angle so that it maintains the same angle with the IKed foot	

	// Project everything onto the plane defined by the axis between the hip and the foot. The knee can be rotated
	// around this axis without changing the position of the effector (the foot).	
	
	// Define each plane	
	FVector HipFootAxisPre    = FootCSPre - HipCSPre;
	if (!HipFootAxisPre.Normalize())
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Knee Correction - HipFootAxisPre Normalization Failure"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		HipFootAxisPre        = FVector(0.0f, 0.0f, 1.0f);
	}
	FVector CenterPre         = HipCSPre + (KneeCSPre - HipCSPre).ProjectOnToNormal(HipFootAxisPre);
	FVector KneeDirectionPre  = (KneeCSPre - CenterPre).GetUnsafeNormal();

	FVector HipFootAxisPost   = FootCSPost - HipCSPost;
	if (!HipFootAxisPost.Normalize())
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Knee Correction - HipFootAxisPost Normalization Failure"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		HipFootAxisPost       = FVector(0.0f, 0.0f, 1.0f);
	}
	FVector CenterPost        = HipCSPost + (KneeCSPost - HipCSPost).ProjectOnToNormal(HipFootAxisPost);
	FVector KneeDirectionPost = (KneeCSPost - CenterPost).GetUnsafeNormal();
	
	// Get the projected foot-toe vectors
	FVector FootToePre = FVector::VectorPlaneProject((ToeCSPre - FootCSPre), HipFootAxisPre);
	if (!FootToePre.Normalize())
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Knee Correction - FootToePre Normalization Failure"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		FootToePre = KneeDirectionPre;
	}

	// Rotate the foot according to how the hip-foot axis is changed. Without this, the foot direction
	// may be reversed when projected onto the rotation plane	
	float HipAxisRad = FMath::Acos(FVector::DotProduct(HipFootAxisPre, HipFootAxisPost));
	FVector FootToeRotationAxis = FVector::CrossProduct(HipFootAxisPre, HipFootAxisPost);
	FVector FootCSPostRotated = FootCSPost;
	FVector ToeCSPostRotated = ToeCSPost;
	if (FootToeRotationAxis.Normalize())
	{
		FQuat FootToeRotation(FootToeRotationAxis, HipAxisRad);
		FVector FootDirection = FootCSPost - HipCSPost;
		FVector ToeDirection = ToeCSPost - HipCSPost;
		FootCSPostRotated = HipCSPost + FootToeRotation.RotateVector(FootDirection);
		ToeCSPostRotated = HipCSPost + FootToeRotation.RotateVector(ToeDirection);
	}
   	
	FVector FootToePost = FVector::VectorPlaneProject((ToeCSPostRotated - FootCSPostRotated), HipFootAxisPost);
	if (!FootToePost.Normalize())
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogRTIK, Warning, TEXT("Knee Correction - FootToePost Normalization Failure"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		FootToePost = KneeDirectionPost;
	}

	// No need to failsafe -- we've already checked that the leg isn't completely straight
	FVector KneePre = (KneeCSPre - CenterPre).GetUnsafeNormal();
	
	// Rotate the post-IK foot to find the corrected knee direction (on the hip-foot plane)
	float FootKneeRad  = FMath::Acos(FVector::DotProduct(FootToePre, KneePre));
	FVector RotationAxis = FVector::CrossProduct(FootToePre, KneePre);

	if (!RotationAxis.Normalize())
	{		

#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogRTIK, Warning, TEXT("Knee correction -- rotation Axis normalization failure "));
#endif

		if (FVector::DotProduct(FootToePre, KneePre) < 0.0f)
		{
			// Knee and foot point in opposite directions
			RotationAxis  = HipFootAxisPost;
			FootKneeRad = 180.0f;
		}
		else
		{
			// Foot any knee point in same direction (no rotation needed)
			RotationAxis  = HipFootAxisPost;
			FootKneeRad = 0.0f;
		}
	}

	FQuat FootKneeRotPost    = FQuat(RotationAxis, FootKneeRad);
	FVector NewKneeDirection = FootKneeRotPost.RotateVector(FootToePost);
	
	// Transform back to component space
	FVector NewKneeCS = CenterPost + (NewKneeDirection * (KneeCSPost - CenterPost).Size());

	// Update rotations for thigh and shin bones
	FVector NewThighVec          = (NewKneeCS - HipCSPost).GetUnsafeNormal();
	FVector NewShinVec           = (FootCSPost - NewKneeCS).GetUnsafeNormal();
	
	FQuat NewHipRotation         = FQuat::FindBetweenNormals(OldThighVec, NewThighVec);
	FQuat NewThighRotation       = FQuat::FindBetweenNormals(OldShinVec, NewShinVec);

	FTransform NewHipTransform   = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, Leg->Chain.HipBone.BoneIndex);
	FTransform NewThighTransform = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, Leg->Chain.ThighBone.BoneIndex);

	NewHipTransform.SetRotation(NewHipRotation*NewHipTransform.GetRotation());
	NewThighTransform.SetRotation(NewThighRotation*NewThighTransform.GetRotation());
	NewThighTransform.SetLocation(NewKneeCS);

	// Update the shin transform, otherwise its component space rotation will change (messing up rotation of the foot)
	FTransform NewShinTransform = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, Leg->Chain.ShinBone.BoneIndex);

	OutBoneTransforms.Add(FBoneTransform(Leg->Chain.HipBone.BoneIndex, NewHipTransform));
	OutBoneTransforms.Add(FBoneTransform(Leg->Chain.ThighBone.BoneIndex, NewThighTransform));
	OutBoneTransforms.Add(FBoneTransform(Leg->Chain.ShinBone.BoneIndex, NewShinTransform));

#if WITH_EDITOR
	if (bEnableDebugDraw)
	{
		UWorld* World = SkelComp->GetWorld();
		FMatrix ToWorld = SkelComp->GetComponentToWorld().ToMatrixNoScale();

		// Draw the pre-IK leg, in red
		FDebugDrawUtil::DrawBoneChain(World, *SkelComp, BasePose.Pose, 
			Leg->Chain.FootBone.BoneIndex, Leg->Chain.HipBone.BoneIndex,
			FColor(255, 0, 0));

		FVector PrePlaneBase = ToWorld.TransformPosition(CenterPre);
		FVector PrePlaneNormal = ToWorld.TransformVector(HipFootAxisPre);
		FDebugDrawUtil::DrawPlane(World, PrePlaneBase, PrePlaneNormal, 100.0f, FColor(255, 0, 255, 40), true);
		FDebugDrawUtil::DrawVector(World, PrePlaneBase, ToWorld.TransformVector(KneePre), FColor(255, 0, 255));
		FDebugDrawUtil::DrawVector(World, PrePlaneBase, ToWorld.TransformVector(FootToePre), FColor(255, 0, 0));
		FDebugDrawUtil::DrawLine(World, ToWorld.TransformPosition(HipCSPre), 
			ToWorld.TransformPosition(HipCSPre) + (PrePlaneNormal * (FootCSPre - HipCSPre).Size()),
			FColor(255, 0, 0));

		
		// Draw post-IK leg, in blue
		FDebugDrawUtil::DrawBoneChain(World, *SkelComp, Output.Pose, 
			Leg->Chain.FootBone.BoneIndex, Leg->Chain.HipBone.BoneIndex,
			FColor(0, 0, 255));

		FVector PostPlaneBase = ToWorld.TransformPosition(CenterPost);
		FVector PostPlaneNormal = ToWorld.TransformVector(HipFootAxisPost);
		FDebugDrawUtil::DrawPlane(World, PostPlaneBase, PostPlaneNormal, 100.0f, FColor(0, 255, 255, 40), true);
		FDebugDrawUtil::DrawLine(World, ToWorld.TransformPosition(HipCSPost), 
			ToWorld.TransformPosition(HipCSPost) + (PostPlaneNormal * (FootCSPost - HipCSPost).Size()),
			FColor(0, 255, 0));

		// Draw the post-correction leg, in green
		FCSPose<FCompactPose> CopiedPose;
		CopiedPose.CopyPose(Output.Pose);
		const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);
		CopiedPose.LocalBlendCSBoneTransforms(OutBoneTransforms, BlendWeight);
		FDebugDrawUtil::DrawBoneChain(World, *SkelComp, CopiedPose, 
			Leg->Chain.FootBone.BoneIndex, Leg->Chain.HipBone.BoneIndex,
			FColor(0, 255, 0));
		
		FDebugDrawUtil::DrawVector(World, PostPlaneBase, ToWorld.TransformVector(NewKneeDirection), FColor(0, 255, 255));
		FDebugDrawUtil::DrawVector(World, PostPlaneBase, ToWorld.TransformVector(FootToePost), FColor(0, 0, 255));
	}
#endif // WITH_EDITOR
	
}

bool FAnimNode_HumanoidLegIKKneeCorrection::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogRTIK, Warning, TEXT("IK Node Humanoid IK Leg Knee Correction was not valid to evaluate -- an input wrapper object was null"));		
#endif // ENABLE_IK_DEBUG_VERBOSE
		return false;
	}
	
	bool bValid = Leg->InitIfInvalid(RequiredBones);

#if ENABLE_IK_DEBUG_VERBOSE
	if (!bValid)
	{
		UE_LOG(LogRTIK, Warning, TEXT("IK Node Humanoid IK Leg Knee Correction was not valid to evaluate"));
	}
#endif // ENABLE_IK_DEBUG_VERBOSE
	
	return bValid;
}

void FAnimNode_HumanoidLegIKKneeCorrection::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{

	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialize Humanoid IK Leg Knee Correction -- An input wrapper object was null"));
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





