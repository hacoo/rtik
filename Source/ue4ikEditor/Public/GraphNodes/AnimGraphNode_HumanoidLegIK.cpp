// Copyright (c) Henry Cooney 2017

#include "AnimGraphNode_HumanoidLegIK.h"

UAnimGraphNode_HumanoidLegIK::UAnimGraphNode_HumanoidLegIK(const FObjectInitializer& ObjectInitializer)
	:
	Super(ObjectInitializer)
{

}

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
