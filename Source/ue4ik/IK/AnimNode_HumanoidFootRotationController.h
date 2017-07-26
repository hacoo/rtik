// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "HumanoidIK.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_HumanoidFootRotationController.generated.h"

/*
  * IKs a humanoid biped leg onto a target location. Should be preceeded by hip adjustment to ensure the legs can reach. 
  * Uses FABRIK IK solver.  
  * 
  * Knee rotation is not enforced in this node.
*/
USTRUCT()
struct UE4IK_API FAnimNode_HumanoidFootRotationController : public FAnimNode_SkeletalControlBase
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

	// How quickly the foot rotates. Increase for closer tracking and less clipping, but snappier movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinHiddenByDefault))
	float RotationSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnableDebugDraw;
   
public:

	FAnimNode_HumanoidFootRotationController()
		:
		bEnableDebugDraw(false),
		DeltaTime(0.0f),
		LastRotationOffset(FQuat::Identity),
		RotationSpeed(20.0f)
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
	FQuat LastRotationOffset;
};
