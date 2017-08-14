// Copyright (c) Henry Cooney 2017

#include "rtikEditor.h"
#include "AnimGraphNode_HumanoidLegIK.h"

FText UAnimGraphNode_HumanoidLegIK::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString("Humanoid Leg IK Solver"));
}

FLinearColor UAnimGraphNode_HumanoidLegIK::GetNodeTitleColor() const
{
	return FLinearColor(0, 1, 1, 1);
}

FString UAnimGraphNode_HumanoidLegIK::GetNodeCategory() const
{
	return FString("IK Nodes");
}

FText UAnimGraphNode_HumanoidLegIK::GetControllerDescription() const
{
	return FText::FromString(FString("IK a humanoid two-bone leg to a location"));
}
