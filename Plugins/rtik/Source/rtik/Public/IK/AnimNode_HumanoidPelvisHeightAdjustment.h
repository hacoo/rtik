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
 * - Assumes that the animroot rests on the floor. 
 * - Traces to the floor from each foot. Each foot 'should' be the same height above the floor impact point as they
 *   are above the animroot.
 * - Moves the hips so that the lowest foot above the lowest floor point is at target height. 
 *   IK will need to be applied to the other foot (it will now be clipping through the ground if the ground is not 
 *   flat).
 */
USTRUCT()
struct RTIK_API FAnimNode_HumanoidPelvisHeightAdjustment : public FAnimNode_SkeletalControlBase
{

	GENERATED_USTRUCT_BODY()

public:
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UHumanoidLegChain_Wrapper* LeftLeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UHumanoidLegChain_Wrapper* RightLeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace, meta = (PinShownByDefault))
	UHumanoidIKTraceData_Wrapper* LeftLegTraceData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace, meta = (PinShownByDefault))
	UHumanoidIKTraceData_Wrapper* RightLegTraceData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UIKBoneWrapper* PelvisBone;

	// How quickly the pelvis moves to match floor height. Set higher to make IK more responsive and prevent
    // floating/sinking feet; setting it too high will cause popping.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	float PelvisAdjustVelocity;

	// Maximum height above the floor to do pelvis adjustment. Will transition back to base pose if the 
	// required hip adjustment is larger than this value. Should probably be something 1 / 3 character capsule height
    // (more if you're brave)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinShownByDefault))
	float MaxPelvisAdjustSize;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnableDebugDraw;

public:

	FAnimNode_HumanoidPelvisHeightAdjustment()
		:
		PelvisAdjustVelocity(150.0f),
		MaxPelvisAdjustSize(50.0),
		bEnableDebugDraw(false),
		DeltaTime(0.0f),
		LastPelvisOffset(0.0f, 0.0f, 0.0f)
	{ }

	// FAnimNode_SkeletalControlBase Interface
	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	//virtual void EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Output) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End FAnimNode_SkeletalControlBase Interface

protected:
	float DeltaTime;
	FVector LastPelvisOffset;
};
