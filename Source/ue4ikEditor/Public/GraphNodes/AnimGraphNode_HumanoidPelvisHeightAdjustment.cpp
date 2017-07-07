// Copyright (c) Henry Cooney 2017

#include "AnimGraphNode_HumanoidPelvisHeightAdjustment.h"

UAnimGraphNode_HumanoidPelvisHeightAdjustment::UAnimGraphNode_HumanoidPelvisHeightAdjustment(const FObjectInitializer& ObjectInitializer)
	:
	Super(ObjectInitializer)
{

}

FText UAnimGraphNode_HumanoidPelvisHeightAdjustment::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString("IK Biped Hip Adjustment"));
}

FLinearColor UAnimGraphNode_HumanoidPelvisHeightAdjustment::GetNodeTitleColor() const
{
	return FLinearColor(0, 1, 1, 1);
}

FString UAnimGraphNode_HumanoidPelvisHeightAdjustment::GetNodeCategory() const
{
	return FString("IK Nodes");
}

FText UAnimGraphNode_HumanoidPelvisHeightAdjustment::GetControllerDescription() const
{
	return FText::FromString(FString("Adjusts the hips and pelvis so legs can reach the floor during IK"));
}
