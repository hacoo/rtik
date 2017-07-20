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
	
	// The leg to trace from
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UHumanoidLegChain_Wrapper* Leg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UIKBoneWrapper* PelvisBone;

	// The trace data object to fill in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace, meta = (PinShownByDefault))
	UHumanoidIKTraceData_Wrapper* TraceData;

	// Maximum height above the floor to do pelvis adjustment. Will transition back to base pose if the 
	// required hip adjustment is larger than this value. Should probably be something 1 / 3 character capsule height
    // (more if you're brave)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinShownByDefault))
	float MaxPelvisAdjustSize;
   
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnableDebugDraw;

public:

	FAnimNode_IKHumanoidLegTrace()
		:
		bEnableDebugDraw(false),
		MaxPelvisAdjustSize(40.0f)
	{ }

	// FAnimNode_Base interface
	virtual void Update(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateComponentSpace(FComponentSpacePoseContext& Output) override;
	virtual void Initialize(const FAnimationInitializeContext& Context) override;
	// End FAnimNode_Base Interface

	virtual bool IsValidToEvaluate(const FBoneContainer & RequiredBones);
};
