// Copyright (c) Henry Cooney 2017

#pragma once

#include "AnimGraphNode_SkeletalControlBase.h"
#include "IK/AnimNode_IKHumanoidLegTrace.h"
#include "AnimGraphNode_IKHumanoidLegTrace.generated.h"

UCLASS()
class RTIKEDITOR_API UAnimGraphNode_IKHumanoidLegTrace : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()
	
public:

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FLinearColor GetNodeTitleColor() const override;
	FString GetNodeCategory() const override;
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }

protected:
	virtual FText GetControllerDescription() const;
protected:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_IKHumanoidLegTrace Node;

};