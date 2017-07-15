// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "HumanoidIK.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_HumanoidLegIK.generated.h"


 /*
 */
USTRUCT()
struct UE4IK_API FAnimNode_HumanoidLegIK : public FAnimNode_SkeletalControlBase
{

	GENERATED_USTRUCT_BODY()

public:
			
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnableDebugDraw;

public:

	FAnimNode_HumanoidLegIK()
		:
		bEnableDebugDraw(false),
		DeltaTime(0.0f)
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

};
