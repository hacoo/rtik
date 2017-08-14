// Copyright(c) Henry Cooney 2017

#include "rtikEditor.h"
#include "AnimGraphNode_RangeLimitedFabrik.h"
#include "Animation/AnimInstance.h"
#include "AnimNodeEditModes.h"


FText UAnimGraphNode_RangeLimitedFabrik::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString("Range Limited FABRIK"));
}

FLinearColor UAnimGraphNode_RangeLimitedFabrik::GetNodeTitleColor() const
{
	return FLinearColor(0, 1, 1, 1);
}

FString UAnimGraphNode_RangeLimitedFabrik::GetNodeCategory() const
{
	return FString("IK Nodes");
}

FText UAnimGraphNode_RangeLimitedFabrik::GetControllerDescription() const
{
	return FText::FromString(FString("FABRIK solver with range limits"));
}
