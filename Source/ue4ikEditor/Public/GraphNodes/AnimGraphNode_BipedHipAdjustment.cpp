// Copyright (c) Henry Cooney 2017

#include "AnimGraphNode_BipedHipAdjustment.h"

UAnimGraphNode_BipedHipAdjustment::UAnimGraphNode_BipedHipAdjustment(const FObjectInitializer& ObjectInitializer)
	:
	Super(ObjectInitializer)
{

}

FText UAnimGraphNode_BipedHipAdjustment::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString("IK Biped Hip Adjustment"));
}

FLinearColor UAnimGraphNode_BipedHipAdjustment::GetNodeTitleColor() const
{
	return FLinearColor(0, 1, 1, 1);
}

FString UAnimGraphNode_BipedHipAdjustment::GetNodeCategory() const
{
	return FString("IK Nodes");
}

FText UAnimGraphNode_BipedHipAdjustment::GetControllerDescription() const
{
	return FText::FromString(FString("Adjusts the hips and pelvis so legs can reach the floor during IK"));
}
