// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "HumanoidIK.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_IKHumanoidLegTrace.generated.h"


/*
  * IKs a humanoid biped leg onto a target location. Should be preceeded by hip adjustment to ensure the legs can reach. 
  * Uses FABRIK IK solver.  
  * 
  * Knee rotation is not enforced in this node.
*/
USTRUCT()
struct UE4IK_API FAnimNode_IKHumanoidLegTrace : public FAnimNode_Base
{
	GENERATED_USTRUCT_BODY()

public:

	// The leg on which IK is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UHumanoidLegChain_Wrapper* Leg;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnableDebugDraw;

public:

	FAnimNode_IKHumanoidLegTrace()
		:
		bEnableDebugDraw(false)
	{ }

	// FAnimNode_Base interface
	virtual void Update(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateComponentSpace(FComponentSpacePoseContext& Output) override;
	virtual void Initialize(const FAnimationInitializeContext& Context) override;
	// End FAnimNode_Base Interface
};
