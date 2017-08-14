// Copyright (c) Henry Cooney 2017

#include "rtikEditor.h"
#include "AnimGraphNode_HumanoidLegIKKneeCorrection.h"

FText UAnimGraphNode_HumanoidLegIKKneeCorrection::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString("Humanoid Leg IK Knee Correction"));
}

FLinearColor UAnimGraphNode_HumanoidLegIKKneeCorrection::GetNodeTitleColor() const
{
	return FLinearColor(0, 1, 1, 1);
}

FString UAnimGraphNode_HumanoidLegIKKneeCorrection::GetNodeCategory() const
{
	return FString("IK Nodes");
}

FText UAnimGraphNode_HumanoidLegIKKneeCorrection::GetControllerDescription() const
{
	return FText::FromString(FString("Corrects knee angle after IK"));
}
