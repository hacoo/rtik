// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "IK.h"
#include "HumanoidIK.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "Engine/SkeletalMeshSocket.h"
#include "AnimNode_HumanoidArmTorsoAdjust.generated.h"

/*
* This node adjusts the torso before arm IK, causing it to bend toward the hand IK targets.
*
* This is accomplished by running FABRIK along each arm chain, allowing the root (the shoulder joint) to 
* be dragged. The dragged shoulder positions are used to determine how the torso should rotate.
*
* The torso will twist (around the spine axis) and pitch (around the skeleton left axis) only; rolling is
* not implemented. 
*
* There are a few important but unintuitive parameters in this node, including:
*
* - MaxShoulderDragDistance - How far the initial FABRIK pass may displace each shoulder bone. Increasing
*   will cause the torso to move more; in particular, faraway targets will have greater weight and the torso 
*   will bend more to reach them. 
* 
* - ShoulderDragStiffness - How much the shoulders will resist being dragged. Increasing this value above 1.0 will
*   make the torso bend less, though it will not limit the maximum bend when reaching for faraway targets.
*   
* - ArmTwistRatio - Since the two arms are IKed separately, each will required a different amount of torso twist.
*   One of these twist rotations will have greater magnitude than the other. This parameter controls the weighting
*   between the two twists; if it is set closer to 1.0, the large-magnitude twist is favored; if it is set closer to
*   0.0, the small-magnitude twist is favored. So, setting this higher will make the torso twist more. You shoulder
*   usually just leave it at 0.5.
*/


/*
* How arm IK should behave
*/
UENUM(BlueprintType)
enum class EHumanoidArmTorsoIKMode : uint8
{	
	// Disabled; return to base pose
	IK_Human_ArmTorso_Disabled UMETA(DisplayName = "Disabled"),
	
	// IK left arm only
	IK_Human_ArmTorso_LeftArmOnly UMETA(DisplayName = "IK left arm only"),

	// IK right arm only
	IK_Human_ArmTorso_RightArmOnly UMETA(DisplayName = "IK right arm only"),

	// IK both arms
	IK_Human_ArmTorso_BothArms UMETA(DisplayName = "IK both arms"),
};


/*
* Rotates the torso and shoulders to prepare for IK.
*/
USTRUCT()
struct UE4IK_API FAnimNode_HumanoidArmTorsoAdjust : public FAnimNode_SkeletalControlBase
{

	GENERATED_USTRUCT_BODY()

public:

	// Pose before any IK or IK pre-processing (e.g., pelvis adjustment) is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FComponentSpacePoseLink BaseComponentPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links, meta = (PinShownByDefault))
	URangeLimitedIKChainWrapper* LeftArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links, meta = (PinShownByDefault))
	URangeLimitedIKChainWrapper* RightArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinShownByDefault))
	EHumanoidArmTorsoIKMode Mode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnableDebugDraw;

	// How precise the FABRIK solver should be. Iteration will cease when effector is within this distance of 
    // the target. Set lower for more accurate IK, but potentially greater cost.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	float Precision;
	
	// Max number of FABRIK iterations. After this many iterations, FABRIK will always stop. Increase for more accurate IK,
    // but potentially greater cost.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	int32 MaxIterations;

	// If set to false, will return to base pose instead of attempting to IK
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	bool bEnable;	

	// Specifies a point that the torso should pivot around during upper-body IK. When the torso
	// bends forward / backward at the waist, this point will remain stationary, and the torso will
	// pivot around it. Should be in the middle of the torso, roughly halfway up the spine.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso)
	FName TorsoPivotSocketName;

	// How far the shoulders may be displaced from their staring position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso)
	float MaxShoulderDragDistance;

	// How much the shoulders will resist being moved from their original positions. Set above 1 to
	// make the shoulders displace less; set below 1 to make them displace more (not recommended)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso, meta = (UIMin=0.0f))
	float ShoulderDragStiffness;
	
	// How far the torso may pitch forward, measured at the waist bone. In positive degrees.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso, meta = (UIMin=0.0f, UIMax = 90.0f))
	float MaxPitchForwardDegrees;

	// How far the torso may pitch backward, measured at the waist bone. In positive degrees.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso, meta = (UIMin = 0.0f, UIMax = 90.0f))
	float MaxPitchBackwardDegrees;

/*
	// How far the torso may move side-to-side, rolling around the forward axis, in either direction. In positive degrees.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso, meta = (UIMin = 0.0f, UIMax = 90.0f))
	float MaxRollDegrees;
*/
	
	// How far the torso may twist, around the character's spine direction, toward the left arm. Measured relative to the incoming animation pose, NOT the character forward direction. In degrees.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso, meta = (UIMin = 0.0f, UIMax = 90.0f))
	float MaxTwistDegreesLeft;

	// How far the torso may twist, around the character's spine direction, toward the right arm. Measured relative to the incoming animation pose, NOT the character forward direction. In degrees.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso, meta = (UIMin = 0.0f, UIMax = 90.0f))
	float MaxTwistDegreesRight;

	// The two arms will require different amounts of twist. If set to 1.0, the larger of the two twists is used;
	// if set to 0.0, the smaller twist is used. Increasing will cause the torso to twist more. You should usually leave this at 0.5.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso, meta = (UIMin = 0.0f, UIMax = 1.0f))
	float ArmTwistRatio;

	// Forward direction for this skeleton. Usually X axis, may sometimes be the Y axis.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso)
	EIKBoneAxis SkeletonForwardAxis;

	// Up direction for this skeleton. Should almost always be the Z axis.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso)
	EIKBoneAxis SkeletonUpAxis;

	// The waist bone indicates where the character will bend at the waist. Usually this is the first spine bone (the one closes to the pelvis)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Torso)
	FIKBone WaistBone;

	// Where to place left arm effector, in world space
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinShownByDefault))
	FTransform LeftArmWorldTarget;

	// Where to place right arm effector, in world space
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinShownByDefault))
	FTransform RightArmWorldTarget;

	// How to handle rotation of the effector (the the hand). If set to No Change, the foot will maintain the same
	// rotation as before IK. If set to Maintain Local, it will maintain the same rotation relative to the parent
	// as before IK. Copy Target Rotation is the same as No Change for now.	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Effector, meta = (PinHiddenByDefault))
	TEnumAsByte<EBoneRotationSource> EffectorRotationSource;

	// How quickly the effector moves toward the target. This parameter is used only is Effector Moves Instantly is set to false.
	// Increase to make IK more responsive but snappier. Uses constant interpolation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Effector)
	float EffectorVelocity;

	// If true, the effector will snap instantly to the target location. If false, the effector will
	// move smoothly, according to EffectorVelocity. Setting to true will make IK responsive but quite snappy. 
	// For most applications, you should probably set this to false.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Effector)
	bool bEffectorMovesInstantly;

public:

	FAnimNode_HumanoidArmTorsoAdjust()
		:
		Mode(EHumanoidArmTorsoIKMode::IK_Human_ArmTorso_Disabled),
		bEnableDebugDraw(false),
		DeltaTime(0.0f),
		Precision(0.001f),
		MaxIterations(10),
		bEnable(true),
		TorsoPivotSocketName(NAME_None),
		MaxShoulderDragDistance(50.0f),
		ShoulderDragStiffness(1.0f),
		MaxPitchForwardDegrees(60.0f),
		MaxPitchBackwardDegrees(10.0f),
		MaxTwistDegreesLeft(30.0f),
		MaxTwistDegreesRight(30.0f),		
		ArmTwistRatio(0.5f),
		SkeletonForwardAxis(EIKBoneAxis::IKBA_X),
		SkeletonUpAxis(EIKBoneAxis::IKBA_Z),
		LeftArmWorldTarget(FVector(0.0f, 0.0f, 0.0f)),
		RightArmWorldTarget(FVector(0.0f, 0.0f, 0.0f)),
		EffectorRotationSource(EBoneRotationSource::BRS_KeepComponentSpaceRotation),
		EffectorVelocity(50.0f),
		bEffectorMovesInstantly(false),
		LastEffectorOffset(0.0f, 0.0f, 0.0f)
	{ }

	// FAnimNode_SkeletalControlBase Interface
	virtual void Initialize(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones(const FAnimationCacheBonesContext& Context) override;

	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	//virtual void EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Output) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End FAnimNode_SkeletalControlBase Interface

protected:
	float DeltaTime;
	FVector LastEffectorOffset;
};
