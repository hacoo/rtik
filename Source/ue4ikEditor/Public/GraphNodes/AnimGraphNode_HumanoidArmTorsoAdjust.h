// Copyright (c) Henry Cooney 2017

#pragma once

#include "AnimGraphNode_SkeletalControlBase.h"
#include "IK/AnimNode_HumanoidArmTorsoAdjust.h"
#include "AnimGraphNode_HumanoidArmTorsoAdjust.generated.h"

UCLASS(MinimalAPI)
class UAnimGraphNode_HumanoidArmTorsoAdjust : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()
	
public:

	UAnimGraphNode_HumanoidArmTorsoAdjust(const FObjectInitializer& ObjectInitializer);
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FLinearColor GetNodeTitleColor() const override;
	FString GetNodeCategory() const override;

protected:
	virtual FText GetControllerDescription() const;
protected:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_HumanoidArmTorsoAdjust Node;

};