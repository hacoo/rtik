// Copyright (c) Henry Cooney 2017

#include "rtikEditor.h"
#include "AnimGraphNode_HumanoidFootRotationController.h"

FText UAnimGraphNode_HumanoidFootRotationController::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString("Humanoid Foot Rotation Controller"));
}

FLinearColor UAnimGraphNode_HumanoidFootRotationController::GetNodeTitleColor() const
{
	return FLinearColor(0, 1, 1, 1);
}

FString UAnimGraphNode_HumanoidFootRotationController::GetNodeCategory() const
{
	return FString("IK Nodes");
}

FText UAnimGraphNode_HumanoidFootRotationController::GetControllerDescription() const
{
	return FText::FromString(FString("Rotate a humanoid's foot to match the slope of the floor"));
}
