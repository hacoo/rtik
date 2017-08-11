// Copyright (c) Henry Cooney 2017

#include "AnimGraphNode_HumanoidLegIKKneeCorrection.h"

UAnimGraphNode_HumanoidLegIKKneeCorrection::UAnimGraphNode_HumanoidLegIKKneeCorrection(const FObjectInitializer& ObjectInitializer)
	:
	Super(ObjectInitializer)
{

}

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
