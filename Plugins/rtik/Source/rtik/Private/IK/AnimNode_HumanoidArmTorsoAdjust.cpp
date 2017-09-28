// Copyright(c) Henry Cooney 2017

#include "rtik.h"
#include "AnimNode_HumanoidArmTorsoAdjust.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "IK/Constraints.h"
#include "IK/RangeLimitedFABRIK.h"
#include "Utility/AnimUtil.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Arm Torso Adjust"), STAT_HumanoidArmTorsoAdjust_Eval, STATGROUP_Anim);

void FAnimNode_HumanoidArmTorsoAdjust::UpdateInternal(const FAnimationUpdateContext & Context)
{
	DeltaTime = Context.GetDeltaTime();	
}


void FAnimNode_HumanoidArmTorsoAdjust::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_HumanoidArmTorsoAdjust_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

	// Input pin pointers are checked in IsValid -- don't need to check here

	USkeletalMeshComponent* SkelComp   = Output.AnimInstanceProxy->GetSkelMeshComponent();
	FMatrix ToCS = SkelComp->GetComponentToWorld().ToMatrixNoScale().Inverse();

/*
	const USkeletalMeshSocket* TorsoPivotSocket = SkelComp->GetSocketByName(TorsoPivotSocketName);
	if (TorsoPivotSocket == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogRTIK, Warning, TEXT("Could not evaluate humanoid arm torso adjustment -- torso pivot socket named %s could not be found"),
			*TorsoPivotSocketName.ToString());
#endif
		return;
	}
*/

	int32 NumBonesLeft = LeftArm->Chain.Num();
	int32 NumBonesRight = RightArm->Chain.Num();
	if (NumBonesLeft < 1 || NumBonesRight < 1)
	{
		return;
	}

	// Get skeleton axes
	FVector ForwardAxis = FIKUtil::IKBoneAxisToVector(SkeletonForwardAxis);
	FVector UpAxis      = FIKUtil::IKBoneAxisToVector(SkeletonUpAxis);
	FVector LeftAxis    = FVector::CrossProduct(ForwardAxis, UpAxis);
	FVector RightAxis   = -1 * LeftAxis;
	
	// Not an actual bone location. The torso will pivot around this location during forward / backward bends.
	// FTransform PivotCS(ToCS.TransformPosition(TorsoPivotSocket->GetSocketLocation(SkelComp)));

	// Upper body rotations are applied at this bone.
	FTransform WaistCS = Output.Pose.GetComponentSpaceTransform(WaistBone.BoneIndex);

#if ENABLE_IK_DEBUG
	if (!LeftAxis.IsNormalized())
	{
		UE_LOG(LogRTIK, Warning, TEXT("Could not evaluate Humanoid Arm Torso Adjustment - Skeleton Forward Axis and Skeleton Up Axis were not orthogonal"));
		return;
	}
#endif 

	// Setup starting transforms
	TArray<FTransform> CSTransformsLeft;
	CSTransformsLeft.Reserve(NumBonesLeft);
	for (FIKBone& Bone : LeftArm->Chain.BonesRootToEffector)
	{
		CSTransformsLeft.Add(Output.Pose.GetComponentSpaceTransform(Bone.BoneIndex));
	}

	TArray<FTransform> CSTransformsRight;
	CSTransformsLeft.Reserve(NumBonesRight);
	for (FIKBone& Bone : RightArm->Chain.BonesRootToEffector)
	{
		CSTransformsRight.Add(Output.Pose.GetComponentSpaceTransform(Bone.BoneIndex));
	}

	// Setup constraints
	TArray<FIKBoneConstraint*> ConstraintsLeft;
	ConstraintsLeft.Reserve(NumBonesLeft);
	for (FIKBone& Bone : LeftArm->Chain.BonesRootToEffector)
	{
		ConstraintsLeft.Add(Bone.GetConstraint());
	}

	TArray<FIKBoneConstraint*> ConstraintsRight;
	ConstraintsRight.Reserve(NumBonesLeft);
	for (FIKBone& Bone : RightArm->Chain.BonesRootToEffector)
	{
		ConstraintsRight.Add(Bone.GetConstraint());
	}

	// First pass: IK each arm, allowing shoulders to drag
	TArray<FTransform> PostIKTransformsLeft;
	if (Mode == EHumanoidArmTorsoIKMode::IK_Human_ArmTorso_BothArms ||
		Mode == EHumanoidArmTorsoIKMode::IK_Human_ArmTorso_LeftArmOnly)
	{
		FVector LeftTargetCS = ToCS.TransformPosition(LeftArmWorldTarget.GetLocation());
		FRangeLimitedFABRIK::SolveRangeLimitedFABRIK(
			CSTransformsLeft,
			ConstraintsLeft,
			LeftTargetCS,
			PostIKTransformsLeft,
			MaxShoulderDragDistance,
			ShoulderDragStiffness,
			Precision,
			MaxIterations,
			Cast<ACharacter>(SkelComp->GetOwner())
		);
	}
	else
	{
		for (FTransform& Transform : CSTransformsLeft)
		{
			PostIKTransformsLeft.Add(Transform);
		}
	}
	TArray<FTransform> PostIKTransformsRight;
	if (Mode == EHumanoidArmTorsoIKMode::IK_Human_ArmTorso_BothArms ||
		Mode == EHumanoidArmTorsoIKMode::IK_Human_ArmTorso_RightArmOnly)
	{
		FVector RightTargetCS = ToCS.TransformPosition(RightArmWorldTarget.GetLocation());
		FRangeLimitedFABRIK::SolveRangeLimitedFABRIK(
			CSTransformsRight,
			ConstraintsRight,
			RightTargetCS,
			PostIKTransformsRight,
			MaxShoulderDragDistance,
			ShoulderDragStiffness,
			Precision,
			MaxIterations,
			Cast<ACharacter>(SkelComp->GetOwner())
		);
	}
	else
	{
		for (FTransform& Transform : CSTransformsRight)
		{
			PostIKTransformsRight.Add(Transform);
		}
	}

	FTransform WaistCSPostIK = WaistCS;

/*
	// Readjust shoulders, and allow some waist movement, using noisy-three-point method
	if (Mode != EHumanoidArmTorsoIKMode::IK_Human_ArmTorso_Disabled)
	{
		FNoisyThreePointClosedLoop InClosedLoop(
			PostIKTransformsLeft[0],
			PostIKTransformsRight[0],
			WaistCS,
			FVector::Dist(CSTransformsLeft[0].GetLocation(), WaistCS.GetLocation()),
			FVector::Dist(CSTransformsRight[0].GetLocation(), WaistCS.GetLocation()),
			FVector::Dist(CSTransformsLeft[0].GetLocation(), CSTransformsRight[0].GetLocation())
		);

		FNoisyThreePointClosedLoop OutClosedLoop;
		
		FRangeLimitedFABRIK::SolveNoisyThreePoint(
			InClosedLoop,
			PostIKTransformsLeft[1],
			PostIKTransformsRight[1],
			OutClosedLoop,
			10.0f,
			ShoulderDragStiffness,
			Precision,
			MaxIterations,
			nullptr
		);
	
		PostIKTransformsLeft[0].SetLocation(OutClosedLoop.EffectorATransform.GetLocation());
		PostIKTransformsRight[0].SetLocation(OutClosedLoop.EffectorBTransform.GetLocation());		
		WaistCSPostIK.SetLocation(OutClosedLoop.RootTransform.GetLocation());
	}
*/

	// Use first pass results to twist the torso around the spine direction
	// Note --calculations here are relative to the waist bone, not root!
	// 'Neck' is the midpoint beween the shoulders
	FVector ShoulderLeftPreIK = CSTransformsLeft[0].GetLocation() - WaistCS.GetLocation();
	FVector ShoulderRightPreIK = CSTransformsRight[0].GetLocation() - WaistCS.GetLocation();
	FVector NeckPreIK = (ShoulderLeftPreIK + ShoulderRightPreIK) / 2;
	FVector SpineDirection = NeckPreIK.GetUnsafeNormal();

	FVector ShoulderLeftPostIK = PostIKTransformsLeft[0].GetLocation() - WaistCSPostIK.GetLocation();
	FVector ShoulderRightPostIK = PostIKTransformsRight[0].GetLocation() - WaistCSPostIK.GetLocation();
	FVector NeckPostIK = (ShoulderLeftPostIK + ShoulderRightPostIK) / 2;
	FVector SpineDirectionPost = NeckPostIK.GetUnsafeNormal();
		
	FVector ShoulderLeftPostIKDir = (ShoulderLeftPostIK - NeckPostIK).GetUnsafeNormal();
	FVector ShoulderRightPostIKDir = (ShoulderRightPostIK - NeckPostIK).GetUnsafeNormal();

	FVector ShoulderLeftPreIKDir = (ShoulderLeftPreIK - NeckPreIK).GetUnsafeNormal();
	FVector ShoulderRightPreIKDir = (ShoulderRightPreIK - NeckPreIK).GetUnsafeNormal();

	// Find the twist angle by blending the small rotation and the large one as specified
	FVector LeftTwistAxis = FVector::CrossProduct(ShoulderLeftPreIKDir, ShoulderLeftPostIKDir);
	float LeftTwistRad = 0.0f;
	if (LeftTwistAxis.Normalize())
	{
		LeftTwistRad = (FVector::DotProduct(LeftTwistAxis, SpineDirection) > 0.0f) ?
			FMath::Acos(FVector::DotProduct(ShoulderLeftPreIKDir, ShoulderLeftPostIKDir)) :
			-1 * FMath::Acos(FVector::DotProduct(ShoulderLeftPreIKDir, ShoulderLeftPostIKDir));
	}

	FVector RightTwistAxis = FVector::CrossProduct(ShoulderRightPreIKDir, ShoulderRightPostIKDir);
	float RightTwistRad = 0.0f;
	if (RightTwistAxis.Normalize())
	{
		RightTwistRad = (FVector::DotProduct(RightTwistAxis, SpineDirection) > 0.0f) ?
			FMath::Acos(FVector::DotProduct(ShoulderRightPreIKDir, ShoulderRightPostIKDir)) :
			-1 * FMath::Acos(FVector::DotProduct(ShoulderRightPreIKDir, ShoulderRightPostIKDir));
	}	

	float TwistRad;
	float SmallRad;
	float LargeRad;
	if (FMath::Abs(LeftTwistRad) > FMath::Abs(RightTwistRad))
	{
		SmallRad = RightTwistRad;
		LargeRad = LeftTwistRad;
	}
	else
	{
		SmallRad = LeftTwistRad;
		LargeRad = RightTwistRad;
	}
		
	if (Mode == EHumanoidArmTorsoIKMode::IK_Human_ArmTorso_BothArms)
	{
		// If IKing both arms, blend the twists
		TwistRad = FMath::Lerp(SmallRad, LargeRad, ArmTwistRatio);
	}
	else
	{
		// Otherwise, use the large twist; the other arm isn't iking and shouldn't contribute
		TwistRad = LargeRad;
	}
	
	float TwistDeg = FMath::RadiansToDegrees(TwistRad);
	TwistDeg = FMath::Clamp(TwistDeg, -MaxTwistDegreesLeft, MaxTwistDegreesRight);
	FQuat TwistRotation(SpineDirection, FMath::DegreesToRadians(TwistDeg));
	
	// Prepare pitch (bend forward / backward) rotation
	FVector SpinePitchPreIK = FVector::VectorPlaneProject(SpineDirection, LeftAxis);
	FVector SpinePitchPostIK = FVector::VectorPlaneProject(SpineDirectionPost, LeftAxis);
	
	FVector PitchPositiveDirection = FVector::CrossProduct(SpinePitchPreIK, SpinePitchPostIK);
	float PitchRad = 0.0f;
	if (PitchPositiveDirection.Normalize())
	{
		PitchRad = (FVector::DotProduct(PitchPositiveDirection, RightAxis) >= 0.0f) ?
			FMath::Acos(FVector::DotProduct(SpinePitchPreIK, SpinePitchPostIK)) :
			-1 * FMath::Acos(FVector::DotProduct(SpinePitchPreIK, SpinePitchPostIK));
	}

	PitchRad = FMath::DegreesToRadians(
		FMath::Clamp(FMath::RadiansToDegrees(PitchRad), -MaxPitchBackwardDegrees, MaxPitchForwardDegrees)
	);
	FQuat PitchRotation(RightAxis, PitchRad);

	// Twist needs to be applied first; pitch will modify twist axes and cause a bad rotation
	FQuat TargetOffset = (PitchRotation * TwistRotation);
	// Interpolate rotation
	LastRotationOffset = FQuat::Slerp(LastRotationOffset, TargetOffset, FMath::Clamp(TorsoRotationSlerpSpeed * DeltaTime, 0.0f, 1.0f));

	// Apply new transforms	
	WaistCS.SetRotation((LastRotationOffset * WaistCS.GetRotation()).GetNormalized());
	OutBoneTransforms.Add(FBoneTransform(WaistBone.BoneIndex, WaistCS));

#if WITH_EDITOR
	if (bEnableDebugDraw)
	{
		UWorld* World = SkelComp->GetWorld();		
		FMatrix ToWorld = SkelComp->ComponentToWorld.ToMatrixNoScale();
		FVector WaistLocWorld = ToWorld.TransformPosition(WaistCS.GetLocation());
		FVector WaistLocWorldPostIK = ToWorld.TransformPosition(WaistCSPostIK.GetLocation());
		FVector ParentLoc;
		FVector ChildLoc;

		// Draw chain before adjustment, in yellow
		for (int32 i = 0; i < NumBonesLeft - 1; ++i)
		{
			// Draw each arm
			ParentLoc = ToWorld.TransformPosition(CSTransformsLeft[i].GetLocation());
			ChildLoc = ToWorld.TransformPosition(CSTransformsLeft[i+1].GetLocation());
			FDebugDrawUtil::DrawLine(World, ParentLoc, ChildLoc, FColor(255, 255, 0));
			FDebugDrawUtil::DrawSphere(World, ChildLoc, FColor(255, 255, 0), 3.0f);

			ParentLoc = ToWorld.TransformPosition(CSTransformsRight[i].GetLocation());
			ChildLoc = ToWorld.TransformPosition(CSTransformsRight[i+1].GetLocation());
			FDebugDrawUtil::DrawLine(World, ParentLoc, ChildLoc, FColor(255, 255, 0));
			FDebugDrawUtil::DrawSphere(World, ChildLoc, FColor(255, 255, 0), 3.0f);
		}		
		// Draw torso triangle
		FDebugDrawUtil::DrawLine(World, WaistLocWorld,
			ToWorld.TransformPosition(CSTransformsLeft[0].GetLocation()),
			FColor(255, 255, 0));
		FDebugDrawUtil::DrawLine(World, WaistLocWorld,
			ToWorld.TransformPosition(CSTransformsRight[0].GetLocation()),
			FColor(255, 255, 0));
		FDebugDrawUtil::DrawLine(World,
			ToWorld.TransformPosition(CSTransformsLeft[0].GetLocation()),
			ToWorld.TransformPosition(CSTransformsRight[0].GetLocation()),
			FColor(255, 255, 0));
		FDebugDrawUtil::DrawSphere(World, WaistLocWorld, FColor(255, 255, 0), 3.0f);
		
		FVector NeckPointPre = ToWorld.TransformPosition((CSTransformsLeft[0].GetLocation() + CSTransformsRight[0].GetLocation()) / 2);
		FDebugDrawUtil::DrawLine(World,
			WaistLocWorld,
			NeckPointPre,
			FColor(255, 255, 0));
		FDebugDrawUtil::DrawSphere(World, NeckPointPre, FColor(255, 255, 0), 3.0f);

		// Draw chain after adjustment, in cyan
		for (int32 i = 0; i < NumBonesLeft - 1; ++i)
		{
			// Draw each arm
			ParentLoc = ToWorld.TransformPosition(PostIKTransformsLeft[i].GetLocation());
			ChildLoc = ToWorld.TransformPosition(PostIKTransformsLeft[i+1].GetLocation());
			FDebugDrawUtil::DrawLine(World, ParentLoc, ChildLoc, FColor(0, 255, 255));
			FDebugDrawUtil::DrawSphere(World, ChildLoc, FColor(0, 255, 255), 3.0f);

			ParentLoc = ToWorld.TransformPosition(PostIKTransformsRight[i].GetLocation());
			ChildLoc = ToWorld.TransformPosition(PostIKTransformsRight[i+1].GetLocation());
			FDebugDrawUtil::DrawLine(World, ParentLoc, ChildLoc, FColor(0, 255, 255));
			FDebugDrawUtil::DrawSphere(World, ChildLoc, FColor(0, 255, 255), 3.0f);
		}
		// Draw torso triangle
		FDebugDrawUtil::DrawLine(World, WaistLocWorldPostIK,
			ToWorld.TransformPosition(PostIKTransformsLeft[0].GetLocation()),
			FColor(0, 255, 255));
		FDebugDrawUtil::DrawLine(World, WaistLocWorldPostIK,
			ToWorld.TransformPosition(PostIKTransformsRight[0].GetLocation()),
			FColor(0, 255, 255));
		FDebugDrawUtil::DrawLine(World,
			ToWorld.TransformPosition(PostIKTransformsLeft[0].GetLocation()),
			ToWorld.TransformPosition(PostIKTransformsRight[0].GetLocation()),
			FColor(0, 255, 255));
		FDebugDrawUtil::DrawSphere(World, WaistLocWorldPostIK, FColor(0, 255, 255), 3.0f);

		FVector NeckPointPost = ToWorld.TransformPosition((PostIKTransformsLeft[0].GetLocation() + PostIKTransformsRight[0].GetLocation()) / 2);
		FDebugDrawUtil::DrawLine(World,
			WaistLocWorldPostIK,
			NeckPointPost,
			FColor(0, 255, 255));
		FDebugDrawUtil::DrawSphere(World, NeckPointPost, FColor(0, 255, 255), 3.0f);

		// Draw skeleton axes
		FVector Base = ToWorld.GetOrigin();
		FDebugDrawUtil::DrawVector(World, Base, ToWorld.TransformVector(ForwardAxis), FColor(255, 0, 0));
		FDebugDrawUtil::DrawVector(World, Base, ToWorld.TransformVector(LeftAxis), FColor(0, 255, 0));
		FDebugDrawUtil::DrawVector(World, Base, ToWorld.TransformVector(UpAxis), FColor(0, 0, 255));

	}
#endif // WITH_EDITOR
}

bool FAnimNode_HumanoidArmTorsoAdjust::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	
	if (LeftArm == nullptr || RightArm == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogRTIK, Warning, TEXT("Humaonid Arm Torso Adjust was not valid to evaluate - an input wrapper was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;		
	}

	if (!LeftArm->IsValid(RequiredBones) || !RightArm->IsValid(RequiredBones))
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogRTIK, Warning, TEXT("Humaonid Arm Torso Adjust was not valid to evaluate - an arm chain was not valid"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;				
	}

	if (!WaistBone.IsValid(RequiredBones))
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogRTIK, Warning, TEXT("Humaonid Arm Torso Adjust was not valid to evaluate - Waist bone was not valid"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;				
	}

	return true;
}

void FAnimNode_HumanoidArmTorsoAdjust::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	if (LeftArm == nullptr || RightArm == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Coud not initialize humanoid arm torso adjust - An input wrapper object was null"));
#endif // ENABLE_IK_DEBUG
		return;
	}
	
	if (!LeftArm->InitBoneReferences(RequiredBones) || !RightArm->InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialize an arm chain in humanoid arm torso adjust"));
#endif // ENABLE_IK_DEBUG
		return;
	}

	if (!WaistBone.Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialize waist bone in humanoid arm torso adjust"));
#endif // ENABLE_IK_DEBUG
		return;
	}
}
