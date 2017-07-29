// Copyright(c) Henry Cooney 2017

#include "AnimGraphNode_RangeLimitedFabrik.h"
#include "Animation/AnimInstance.h"
#include "AnimNodeEditModes.h"

#define LOCTEXT_NAMESPACE "RangeLimitedFabrik"

UAnimGraphNode_RangeLimitedFabrik::UAnimGraphNode_RangeLimitedFabrik(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

FText UAnimGraphNode_RangeLimitedFabrik::GetControllerDescription() const
{
	return LOCTEXT("Range limited Fabrik", "Range Limited FABRIK");
}

void UAnimGraphNode_RangeLimitedFabrik::Draw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent * PreviewSkelMeshComp) const
{
	if(PreviewSkelMeshComp)
	{
		if(FAnimNode_RangeLimitedFabrik* ActiveNode = GetActiveInstanceNode<FAnimNode_RangeLimitedFabrik>(PreviewSkelMeshComp->GetAnimInstance()))
		{
			ActiveNode->ConditionalDebugDraw(PDI, PreviewSkelMeshComp);
		}
	}
}

FText UAnimGraphNode_RangeLimitedFabrik::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return GetControllerDescription();
}

void UAnimGraphNode_RangeLimitedFabrik::CopyNodeDataToPreviewNode(FAnimNode_Base* InPreviewNode)
{
	FAnimNode_RangeLimitedFabrik* Fabrik = static_cast<FAnimNode_RangeLimitedFabrik*>(InPreviewNode);

	// copies Pin values from the internal node to get data which are not compiled yet
	Fabrik->EffectorTransform = Node.EffectorTransform;
}

FEditorModeID UAnimGraphNode_RangeLimitedFabrik::GetEditorMode() const
{
	return AnimNodeEditModes::Fabrik;
}

#undef LOCTEXT_NAMESPACE