// Copyright (c) Henry Cooney 2017

#include "rtikEditor.h"
#include "AnimGraphNode_HumanoidArmTorsoAdjust.h"


FText UAnimGraphNode_HumanoidArmTorsoAdjust::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString("IK Humanoid Arm Torso Adjustment"));
}

FLinearColor UAnimGraphNode_HumanoidArmTorsoAdjust::GetNodeTitleColor() const
{
	return FLinearColor(0, 1, 1, 1);
}

FString UAnimGraphNode_HumanoidArmTorsoAdjust::GetNodeCategory() const
{
	return FString("IK Nodes");
}

FText UAnimGraphNode_HumanoidArmTorsoAdjust::GetControllerDescription() const
{
	return FText::FromString(FString("Adjust humanoid torso rotation before IK"));
}
