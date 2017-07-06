// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "IK.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_BipedHipAdjustment.generated.h"

/**
 * 
 */
USTRUCT()
struct UE4IK_API FAnimNode_BipedHipAdjustment : public FAnimNode_SkeletalControlBase
{

	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FPoseLink InputPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IK, meta = (PinShownByDefault))
	FFabrikBipedLegChain LeftLeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IK, meta = (PinShownByDefault))
	FFabrikBipedLegChain RightLeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IK, meta = (PinShownByDefault))
	FIKBone PelvisBone;

public:

	FAnimNode_BipedHipAdjustment()
		:
		DeltaTime(0.0f),
		bInitSuccess(false)
	{ }

	// FAnimNode_SkeletalControlBase Interface
	virtual void UpdateInternal(const FAnimationUpdateContext& Context);
	virtual void EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Context);
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms);
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones);
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones);
	// End FAnimNode_SkeletalControlBase Interface

private:
	float DeltaTime;
	bool bInitSuccess;

};
