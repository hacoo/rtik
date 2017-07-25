// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "HumanoidIK.h"
#include "BoneControllers/AnimNode_Fabrik.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_HumanoidLegIK.generated.h"


/*
* How leg IK should behave
*/
UENUM(BlueprintType)
enum class EHumanoidLegIKMode : uint8
{	
	// IK for normal locomotion -- will prevent feet from clipping or floating above the ground during normal movement.
	IK_Human_Leg_Locomotion UMETA(DisplayName = "Normal Locomotion"),
	
	// IK onto an arbitrary world location
	IK_Human_Leg_WorldLocation UMETA(DisplayName = "IK Onto World Location")
};

/*
  * IKs a humanoid biped leg onto a target location. Should be preceeded by hip adjustment to ensure the legs can reach. 
  * Uses FABRIK IK solver.  
  * 
  * Knee rotation is not enforced in this node.
*/
USTRUCT()
struct UE4IK_API FAnimNode_HumanoidLegIK : public FAnimNode_SkeletalControlBase
{

	GENERATED_USTRUCT_BODY()

public:

	// Pose before any IK or IK pre-processing (e.g., pelvis adjustment) is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FComponentSpacePoseLink BaseComponentPose;

	// The leg on which IK is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UHumanoidLegChain_Wrapper* Leg;

	// Trace data for this leg (use IKHumanoidLegTrace to update it)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UHumanoidIKTraceData_Wrapper* TraceData;
	
	// Target location for the foot; IK will attempt to move the tip of the shin here. In world space.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	FVector FootTargetWorld;
		
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

	// Set to 'locomotion' for normal movement; 'world location' to manually IK the leg onto a world location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver, meta = (PinHiddenByDefault))
	EHumanoidLegIKMode Mode;
	
	// How to handle rotation of the effector (the foot). If set to No Change, the foot will maintain the same
	// rotation as before IK. If set to Maintain Local, it will maintain the same rotation relative to the parent
	// as before IK. Copy Target Rotation is the same as No Change for now.	
	//
	// You should almost certainly set this to No Change and handle foot rotation separately. Change at your own risk!	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver, meta = (PinHiddenByDefault))
	TEnumAsByte<EBoneRotationSource> EffectorRotationSource;

	// How quickly the effector moves toward the target. This parameter is used only is Effector Moves Instantly is set to false.
	// Increase to make IK more responsive but snappier. Uses smooth interpolation, not constant.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float EffectorInterpSpeed;

	// If true, the effector will snap instantly to the target location. If false, the effector will
	// move smoothly, according to EffectorVelocity. Setting to true will make IK responsive but quite snappy. 
	// For most applications, you should probably set this to false.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEffectorMovesInstantly;

   
public:

	FAnimNode_HumanoidLegIK()
		:
		bEnableDebugDraw(false),
		DeltaTime(0.0f),
		FootTargetWorld(0.0f, 0.0f, 0.0f),
		Precision(0.001f),
		MaxIterations(10),
		bEnable(true),
		Mode(EHumanoidLegIKMode::IK_Human_Leg_Locomotion),
		EffectorRotationSource(EBoneRotationSource::BRS_KeepComponentSpaceRotation),
		EffectorInterpSpeed(50.0f),
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
	FAnimNode_Fabrik FabrikSolver;
};
