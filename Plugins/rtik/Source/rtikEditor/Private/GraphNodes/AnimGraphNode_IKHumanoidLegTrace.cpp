// Copyright (c) Henry Cooney 2017

#include "rtikEditor.h"
#include "AnimGraphNode_IKHumanoidLegTrace.h"

FText UAnimGraphNode_IKHumanoidLegTrace::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString("Humanoid IK Leg Trace"));
}

FLinearColor UAnimGraphNode_IKHumanoidLegTrace::GetNodeTitleColor() const
{
	return FLinearColor(0, 1, 1, 1);
}

FString UAnimGraphNode_IKHumanoidLegTrace::GetNodeCategory() const
{
	return FString("IK Nodes");
}

FText UAnimGraphNode_IKHumanoidLegTrace::GetControllerDescription() const
{
	return FText::FromString(FString("Traces from the leg to the floor, providing trace data used later in IK"));
}
