// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "IK.h"
#include "HumanoidIK.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_HumanoidArmTorsoAdjust.generated.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinShownByDefault))
	URangeLimitedIKChainWrapper* Arm;
		
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

	// World-space target to place the effector
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	FVector EffectorWorldTarget;

	// How to handle rotation of the effector (the the hand). If set to No Change, the foot will maintain the same
	// rotation as before IK. If set to Maintain Local, it will maintain the same rotation relative to the parent
	// as before IK. Copy Target Rotation is the same as No Change for now.	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver, meta = (PinHiddenByDefault))
	TEnumAsByte<EBoneRotationSource> EffectorRotationSource;

	// How quickly the effector moves toward the target. This parameter is used only is Effector Moves Instantly is set to false.
	// Increase to make IK more responsive but snappier. Uses constant interpolation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float EffectorVelocity;

	// If true, the effector will snap instantly to the target location. If false, the effector will
	// move smoothly, according to EffectorVelocity. Setting to true will make IK responsive but quite snappy. 
	// For most applications, you should probably set this to false.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEffectorMovesInstantly;

public:

	FAnimNode_HumanoidArmTorsoAdjust()
		:
		bEnableDebugDraw(false),
		DeltaTime(0.0f),
		Precision(0.001f),
		MaxIterations(10),
		bEnable(true),
		EffectorWorldTarget(0.0f, 0.0f, 0.0f),
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
