// Copyright (c) Henry Cooney 2017

#include "Constraints.h"


#pragma region UIKNoBoneConstraint
void UNoBoneConstraint::EnforceConstraint(
	int32 Index,
	const TArray<FTransform>& InCSTransforms,
	const TArray<UIKBoneConstraint*>& Constraints,
	TArray<FTransform>& OutCSTransforms
)
{
	return;
}
#pragma endregion UIKNoBoneConstraint

void UPlanarRotation::EnforceConstraint(
	int32 Index,
	const TArray<FTransform>& InCSTransforms,
	const TArray<UIKBoneConstraint*>& Constraints,
	TArray<FTransform>& OutCSTransforms)
{


	


}
