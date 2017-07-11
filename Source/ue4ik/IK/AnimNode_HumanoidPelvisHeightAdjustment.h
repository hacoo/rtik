// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "HumanoidIK.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_HumanoidPelvisHeightAdjustment.generated.h"


/**
 * Moves the pelvis so the lowest leg can reach the ground. This is an imporant IK pre-processing step;
 * it ensures that both legs are close enough to the ground to actually reach.
 * 
 * Functions as follows:
 * - Checks if foot is 'floating' in the air -- that is, if it is above where it would be in the base animation pose
 * - If so, smoothly transition the hips down so the low foot can reach. The hips are never transitioned up.
 * - If the required hip transition is too large, abort and transition back to the default hip height.
 */
USTRUCT()
struct UE4IK_API FAnimNode_HumanoidPelvisHeightAdjustment : public FAnimNode_SkeletalControlBase
{

	GENERATED_USTRUCT_BODY()

public:
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	FFabrikHumanoidLegChain LeftLeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	FFabrikHumanoidLegChain RightLeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	FIKBone PelvisBone;

	// How quickly the pelvis moves to match floor height. Set higher to make IK more responsive and prevent
    // floating feet; setting it too high will cause popping.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float PelvisAdjustVelocity;

	// Maximum height above the floor to do pelvis adjustment. Will transition back to base pose if 
    // distance between low foot and floor is above this height. Should probably be something 1/3 character capsule height
    // (more if you're brave)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float MaxPelvisAdjustHeight;

public:

	FAnimNode_HumanoidPelvisHeightAdjustment()
		:
		DeltaTime(0.0f),
		LastHipOffset(0.0f, 0.0f, 0.0f),
		PelvisAdjustVelocity(20.0f),
		MaxPelvisAdjustHeight(40.0)
	{ }

	// FAnimNode_SkeletalControlBase Interface
	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	//virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual void EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Output) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End FAnimNode_SkeletalControlBase Interface

protected:
	float DeltaTime;
	FVector LastHipOffset;
};
