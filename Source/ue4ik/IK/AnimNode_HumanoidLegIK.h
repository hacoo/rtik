// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "HumanoidIK.h"
#include "BoneControllers/AnimNode_Fabrik.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_HumanoidLegIK.generated.h"


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

	// The leg on which IK is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	FHumanoidLegChain Leg;
	
	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnable;	

	// How
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	EIKUnreachableRule UnreachableRule;

public:

	FAnimNode_HumanoidLegIK()
		:
		bEnableDebugDraw(false),
		DeltaTime(0.0f),
		FootTargetWorld(0.0f, 0.0f, 0.0f),
		Precision(0.001f),
		MaxIterations(10),
		bEnable(true),
		UnreachableRule(EIKUnreachableRule::IK_Abort)
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
	FAnimNode_Fabrik FabrikSolver;
};
