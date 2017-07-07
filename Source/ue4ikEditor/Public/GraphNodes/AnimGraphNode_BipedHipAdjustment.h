// Copyright (c) Henry Cooney 2017

#pragma once

#include "AnimGraphNode_SkeletalControlBase.h"
#include "IK/AnimNode_BipedHipAdjustment.h"
#include "AnimGraphNode_BipedHipAdjustment.generated.h"

/**
 * Moves the pelvis so the lowest leg can reach the ground. This is an imporant IK pre-processing step;
 * it ensures that both legs are close enough to the ground to actually reach.
 */
UCLASS(MinimalAPI)
class UAnimGraphNode_BipedHipAdjustment : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()
	
public:
	UAnimGraphNode_BipedHipAdjustment(const FObjectInitializer& ObjectInitializer);
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FLinearColor GetNodeTitleColor() const override;
	FString GetNodeCategory() const override;

protected:
	virtual FText GetControllerDescription() const;
protected:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_BipedHipAdjustment Node;
};
