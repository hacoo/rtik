// Copyright(c) Henry Cooney 2017

#include "AnimNode_HumanoidLegIKKneeCorrection.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "Utility/AnimUtil.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Leg IK Eval"), STAT_HumanoidLegIKKneeCorrection_Eval, STATGROUP_Anim);

void FAnimNode_HumanoidLegIKKneeCorrection::Initialize(const FAnimationInitializeContext & Context)
{
	Super::Initialize(Context);
	BaseComponentPose.Initialize(Context);
}

void FAnimNode_HumanoidLegIKKneeCorrection::CacheBones(const FAnimationCacheBonesContext & Context)
{
	Super::CacheBones(Context);
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

		
	// To correct the knee:
	// - Project everything onto the plane normal to the vector from thigh to foot
	// - Find the angle, in the base pose, between the direction of the foot and the direction of the knee. If the leg is fully extended, assume this angle is 0.	
	// - Rotate the IKed knee angle so that it maintains the same angle with the IKed foot	

	// Project everything onto the plane defined by the axis between the hip and the foot. The knee can be rotated
	// around this axis without changing the position of the effector (the foot).	
	
	// Define each plane	
	FVector HipFootAxisPre  = FootCSPre - HipCSPre;
	if (!HipFootAxisPre.Normalize())
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Knee Correction - HipFootAxisPre Normalization Failure"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		HipFootAxisPre      = FVector(0.0f, 0.0f, 1.0f);
	}
	FVector CenterPre       = HipCSPre + (KneeCSPre - HipCSPre).ProjectOnToNormal(HipFootAxisPre);

	FVector HipFootAxisPost = FootCSPost - HipCSPost;
	if (!HipFootAxisPost.Normalize())
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Knee Correction - HipFootAxisPost Normalization Failure"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		HipFootAxisPost     = FVector(0.0f, 0.0f, 1.0f);
	}
	FVector CenterPost      = HipCSPost + (KneeCSPost - HipCSPost).ProjectOnToNormal(HipFootAxisPost);

	// Ensure both axes point in the same direction
	if (FVector::DotProduct(HipFootAxisPre, HipFootAxisPost) < 0.0f)
	{
		HipFootAxisPost *= -1;
	}

	// Get the projected foot-toe vectors
	FVector FootPre     = FVector::PointPlaneProject(FootCSPre, CenterPre, HipFootAxisPre);
	FVector ToePre      = FVector::PointPlaneProject(ToeCSPre, CenterPre, HipFootAxisPre);
	FVector FootToePre  = (ToePre - FootPre);
	if (!FootToePre.Normalize())
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Knee Correction - FootToePre Normalization Failure"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		FootToePre      = FVector(1.0f, 0.0f, 0.0f);
	}
	
	FVector FootPost    = FVector::PointPlaneProject(FootCSPost, CenterPost, HipFootAxisPost);
	FVector ToePost     = FVector::PointPlaneProject(ToeCSPost, CenterPost, HipFootAxisPost);
	FVector FootToePost = (ToePost - FootPost);
	if (!FootToePost.Normalize())
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Knee Correction - FootToePost Normalization Failure"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		FootToePost     = FVector(1.0f, 0.0f, 0.0f);
	}
	
	// Get the projected knee vectors. If the knee vector can't be normalize -- meaning the leg is completely straight, 
	// and there is no knee direction -- failsafe by using the direction of the toe	(i.e, the knee tracks perfectly above the toe)

	FVector KneePre               = KneeCSPre - CenterPre;
	if (!KneePre.Normalize())
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Knee Correction - KneePre Normalization Failure"));
#endif // ENABLE_IK_DEBUG_VERBOSE
		KneePre                   = FootToePre;
	}
	
	FVector KneePost              = KneeCSPost - CenterPost;
	
	FQuat FootKneeRotPre          = FQuat::FindBetweenNormals(FootToePre, KneePre);	

	// Rotate the post-IK foot to find the corrected knee direction (on the hip-foot plane)
	FQuat FootKneeRotPost         = FQuat(HipFootAxisPost,  2 * FMath::Acos(FootKneeRotPre.W));
	FVector NewKneeDirection      = FootKneeRotPost.RotateVector(FootToePost);
	
	// Transform back to component space
	FVector NewKneeCS             = CenterPost + (NewKneeDirection * KneePost.Size());

	// Update rotations for thigh and shin bones
	FVector OldThighVec = (KneeCSPost - HipCSPost).GetUnsafeNormal();
	FVector OldShinVec  = (FootCSPost - KneeCSPost).GetUnsafeNormal();

	FVector NewThighVec = (NewKneeCS - HipCSPost).GetUnsafeNormal();
	FVector NewShinVec  = (FootCSPost - NewKneeCS).GetUnsafeNormal();
	
	FQuat NewHipRotation         = FQuat::FindBetweenNormals(OldThighVec, NewThighVec);
	FQuat NewThighRotation       = FQuat::FindBetweenNormals(OldShinVec, NewShinVec);

	FTransform NewHipTransform   = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, Leg->Chain.HipBone.BoneIndex);
	FTransform NewThighTransform = FAnimUtil::GetBoneCSTransform(*SkelComp, Output.Pose, Leg->Chain.ThighBone.BoneIndex);

	NewHipTransform.SetRotation(NewHipRotation*NewHipTransform.GetRotation());
	NewThighTransform.SetRotation(NewThighRotation*NewThighTransform.GetRotation());
	NewThighTransform.SetLocation(NewKneeCS);

	OutBoneTransforms.Add(FBoneTransform(Leg->Chain.HipBone.BoneIndex, NewHipTransform));
	OutBoneTransforms.Add(FBoneTransform(Leg->Chain.ThighBone.BoneIndex, NewThighTransform));

#if WITH_EDITOR
	if (bEnableDebugDraw)
	{
		UWorld* World = SkelComp->GetWorld();
		FMatrix ToWorld = SkelComp->ComponentToWorld.ToMatrixNoScale();

		// Draw the pre-IK leg, in red
		FDebugDrawUtil::DrawBoneChain(World, *SkelComp, BasePose.Pose, 
			Leg->Chain.FootBone.BoneIndex, Leg->Chain.HipBone.BoneIndex,
			FColor(255, 0, 0));

		FVector PrePlaneBase = ToWorld.TransformPosition(CenterPre);
		FVector PrePlaneNormal = ToWorld.TransformVector(HipFootAxisPre);
		FDebugDrawUtil::DrawPlane(World, PrePlaneBase, PrePlaneNormal, 100.0f, FColor(255, 0, 255, 40));
		
		// Draw post-IK leg, in blue
		FDebugDrawUtil::DrawBoneChain(World, *SkelComp, Output.Pose, 
			Leg->Chain.FootBone.BoneIndex, Leg->Chain.HipBone.BoneIndex,
			FColor(0, 0, 255));

		FVector PostPlaneBase = ToWorld.TransformPosition(CenterPost);
		FVector PostPlaneNormal = ToWorld.TransformVector(HipFootAxisPost);
		FDebugDrawUtil::DrawPlane(World, PostPlaneBase, PostPlaneNormal, 100.0f, FColor(0, 255, 255, 40));

		// Draw the post-correction leg, in green
		FCSPose<FCompactPose> CopiedPose;
		CopiedPose.CopyPose(Output.Pose);
		const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);
		CopiedPose.LocalBlendCSBoneTransforms(OutBoneTransforms, BlendWeight);
		FDebugDrawUtil::DrawBoneChain(World, *SkelComp, CopiedPose, 
			Leg->Chain.FootBone.BoneIndex, Leg->Chain.HipBone.BoneIndex,
			FColor(0, 255, 0));
	}
#endif // WITH_EDITOR
	
}

bool FAnimNode_HumanoidLegIKKneeCorrection::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid IK Leg Knee Correction was not valid to evaluate -- an input wrapper object was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;
	}
	
	bool bValid = Leg->InitIfInvalid(RequiredBones);

#if ENABLE_IK_DEBUG_VERBOSE
	if (!bValid)
	{
		UE_LOG(LogIK, Warning, TEXT("IK Node Humanoid IK Leg Knee Correction was not valid to evaluate"));
	}
#endif // ENABLE_ANIM_DEBUG
	
	return bValid;
}

void FAnimNode_HumanoidLegIKKneeCorrection::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{

	if (Leg == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize Humanoid IK Leg Knee Correction -- An input wrapper object was null"));
#endif // ENABLE_IK_DEBUG

		return;
	}

	if (!Leg->InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize Humanoid IK Leg"));
#endif // ENABLE_IK_DEBUG
	}
}





