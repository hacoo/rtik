// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "HumanoidIK.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_HumanoidLegIK.generated.h"


/*
  * IKs a humanoid biped leg onto a target location. Should be preceeded by hip adjustment to ensure the legs can reach. 
  * Humanoid legs are assumed to have two-bones; therefore this node uses a simple two-bone IK solver.  
  * 
  * Knee orientation is relative to the foot bone. By default, knee will maintain the same orientation, relative the foot
  * bone, as in the base animation pose. This knee constraint can be turned off, or you can mainually set a direction.
  * 
*/
USTRUCT()
struct UE4IK_API FAnimNode_HumanoidLegIK : public FAnimNode_SkeletalControlBase
{

	GENERATED_USTRUCT_BODY()

public:

	// The leg on which IK is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	FHumanoidLegChain Leg;

	// Target location for the foot; IK will attempt to move the tip of the shin here. In world space.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	FVector FootTargetWorld;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnableDebugDraw;

	// If set to false, will return to base pose instead of attempting to IK
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnable;
	

public:

	FAnimNode_HumanoidLegIK()
		:
		bEnableDebugDraw(false),
		DeltaTime(0.0f),
		FootTargetWorld(0.0f, 0.0f, 0.0f),
		bEnable(true)
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
