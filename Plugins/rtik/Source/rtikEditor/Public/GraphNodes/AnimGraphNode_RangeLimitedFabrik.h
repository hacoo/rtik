// Copyright(c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "IK/AnimNode_RangeLimitedFabrik.h"
#include "AnimGraphNode_RangeLimitedFabrik.generated.h"

class FPrimitiveDrawInterface;
class USkeletalMeshComponent;

UCLASS()
class RTIKEDITOR_API UAnimGraphNode_RangeLimitedFabrik : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()

public:

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FLinearColor GetNodeTitleColor() const override;
	FString GetNodeCategory() const override;

protected:
	// UAnimGraphNode_SkeletalControlBase interface
	virtual FText GetControllerDescription() const override;
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_RangeLimitedFabrik Node;
	// End of UAnimGraphNode_SkeletalControlBase interface
};
