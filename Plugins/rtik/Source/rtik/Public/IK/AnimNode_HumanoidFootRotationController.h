// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "HumanoidIK.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_HumanoidFootRotationController.generated.h"

// Rotates the foot to match the floor slope during IK. Will also adjust the foot when it's just
// above the floor, as this looks more natural.
USTRUCT()
struct RTIK_API FAnimNode_HumanoidFootRotationController : public FAnimNode_SkeletalControlBase
{

	GENERATED_USTRUCT_BODY()

public:

	// The leg on which IK is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UHumanoidLegChain_Wrapper* Leg;

	// Trace data for this leg (use IKHumanoidLegTrace to update it)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UHumanoidIKTraceData_Wrapper* TraceData;

	// How quickly the foot rotates (using Slerp). Decrease to keep the foot rotation from snapping. 
	// You can probably set this pretty high -- foot rotation snapping usually isn't that noticable.	
	// Only used is bInterpolateRotation is true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinHiddenByDefault))
	float RotationSlerpSpeed;

	// If true, will smoothly interpolate to the target rotation accoridn to RotationSlerpSpeed.
	// If false, will snap there instantly. 	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bInterpolateRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnableDebugDraw;
   
public:

	FAnimNode_HumanoidFootRotationController()
		:
		RotationSlerpSpeed(20.0f),
		bInterpolateRotation(true),
		bEnableDebugDraw(false),
		DeltaTime(0.0f),
		LastRotationOffset(FQuat::Identity)
	{ }

	// FAnimNode_SkeletalControlBase Interface
	virtual void UpdateInternal(const FAnimationUpdateContext & Context);
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	//virtual void EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Output) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End FAnimNode_SkeletalControlBase Interface

protected:
	float DeltaTime;
	FQuat LastRotationOffset;
};
