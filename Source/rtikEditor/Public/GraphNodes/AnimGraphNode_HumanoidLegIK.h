// Copyright (c) Henry Cooney 2017

#pragma once

#include "AnimGraphNode_SkeletalControlBase.h"
#include "IK/AnimNode_HumanoidLegIK.h"
#include "AnimGraphNode_HumanoidLegIK.generated.h"

UCLASS(MinimalAPI)
class UAnimGraphNode_HumanoidLegIK : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()
	
public:

	UAnimGraphNode_HumanoidLegIK(const FObjectInitializer& ObjectInitializer);
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FLinearColor GetNodeTitleColor() const override;
	FString GetNodeCategory() const override;

protected:
	virtual FText GetControllerDescription() const;
protected:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_HumanoidLegIK Node;

};