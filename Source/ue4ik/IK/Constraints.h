// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "IK.h"
#include "Constraints.generated.h"

/*
	Contains defintions for constraint types used in IK
*/



/*
	The bone is unconstrainted and may move freely.
*/
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class UE4IK_API UNoBoneConstraint : public UIKBoneConstraint
{
	
	GENERATED_BODY()
	
public: 
	virtual void EnforceConstraint(
		int32 Index,
		const TArray<FTransform>& InCSTransforms,
		const TArray<UIKBoneConstraint*>& Constraints,
		TArray<FTransform>& OutCSTransforms
	) override;
};

/*
	The bone may only rotate around the specified Rotation Axis; i.e., it must stay in the plane normal to the rotation axis.	

	Rotation axis is a vector in component space.

	The rotation angle is with respect to a second component-space vector, ForwardDirection. I.e.: if the bone is aligned directly with ForwardDirection, the rotation is 0 degrees. If ForwardDirection is not on the rotation plane, it will be projected onto it, so it must not be normal to the rotation plane.

	Finally, the sign of the rotation angle is defined by the cross product of ForwardDirection and RotationAxis.
*/
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class UE4IK_API UPlanarRotation : public UIKBoneConstraint
{
	GENERATED_BODY()

public:

	// Vector in component space. The bone direction (parent to child) will rotate around this vector, based at the parent.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	EIKBoneAxis RotationAxis = EIKBoneAxis::IKBA_Y;

	// Vector in component space. This represents the '0 degree' rotation; i.e., the bone has a rotation of 0 degrees if the parent-child vector points in this direction. Must not be colinear with RotationAxis.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	EIKBoneAxis ForwardDirection = EIKBoneAxis::IKBA_X;

	// Maximum angle in the positive direction (toward ForwardDirection X RotationAxis), relative to ForwardDirection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (UIMin=0.0f, UIMax=180.0f))
	float MaxDegreesPositive = 45.0f;

	// Maximum angle in the negative direction (away from ForwardDirection X RotationAxis), relative to ForwardDirection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (UIMin=0.0f, UIMax=180.0f))
	float MaxDegreesNegative = 45.0f;

public:
	virtual void EnforceConstraint(
		int32 Index,
		const TArray<FTransform>& InCSTransforms,
		const TArray<UIKBoneConstraint*>& Constraints,
		TArray<FTransform>& OutCSTransforms
	) override;
};
