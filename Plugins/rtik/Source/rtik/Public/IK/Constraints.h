// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "IK.h"
#include "Constraints.generated.h"


// Contains defintions for constraint types used in IK

// The bone is unconstrainted and may move freely.
USTRUCT(BlueprintType)
struct RTIK_API FNoBoneConstraint : public FIKBoneConstraint
{
	
	GENERATED_USTRUCT_BODY()
	
public: 
  
	FNoBoneConstraint() { }

	virtual void EnforceConstraint(
		int32 Index,
		const TArray<FTransform>& ReferenceCSTransforms,
		const TArray<FIKBoneConstraint*>& Constraints,
		TArray<FTransform>& CSTransforms,
		ACharacter* Character = nullptr
	) override;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class UNoBoneConstraintWrapper : public UIKBoneConstraintWrapper
{ 
	GENERATED_BODY()

public: 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FNoBoneConstraint Constraint;

	// Subclasses must override this to return the internal constraint struct
	virtual FIKBoneConstraint* GetConstraint() override { return &Constraint; };
};



// The bone may only rotate around the specified Rotation Axis; i.e., it must stay in the plane normal to the rotation axis.
// 
// Rotation axis is a vector in component space.
// 
// The rotation angle is with respect to a second component - space vector, ForwardDirection.I.e.: if the bone is aligned directly with ForwardDirection, the rotation is 0 degrees.If ForwardDirection is not on the rotation plane, it will be projected onto it, so it must not be normal to the rotation plane.
//
// Finally, the sign of the rotation angle is defined by the cross product of RotationAxis and ForwardDirection.
//
// If the bone direction is normal to the rotation plane, it will be forced to point in FailsafeDirection.
USTRUCT(BlueprintType)
struct RTIK_API FPlanarRotation : public FIKBoneConstraint
{
	GENERATED_USTRUCT_BODY()

public:
  
	// Vector in component space. The bone direction (parent to child) will rotate around this vector, based at the parent. It should be normalized.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FVector RotationAxis;

	// Vector in component space. This represents the '0 degree' rotation; i.e., the bone has a rotation of 0 degrees if the parent-child vector points in this direction. It should be normalized and normal to RotationAxis.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FVector ForwardDirection;

	// Vector in component space. The bone will point in this direction if the constraint method fails (i.e., if the bone direction is normal to the rotation plane). It must be normalized.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FVector FailsafeDirection;

	// Maximum angle in the positive direction (toward RotationAxis X ForwardDirection), relative to ForwardDirection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (UIMin = -180.0f, UIMax = 180.0f))
	float MaxDegrees;

	// Minimum angle in the positive direction (toward RotationAxis X ForwardDirection), relative to ForwardDirection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (UIMin = -180.0f, UIMax = 180.0f))
	float MinDegrees;

public:

	FPlanarRotation()
		:
		RotationAxis(0.0f, 1.0f, 10.0f),
		ForwardDirection(1.0f, 0.0f, 0.0f),
		FailsafeDirection(1.0f, 0.0f, 0.0f),
		MaxDegrees(45.0f),
		MinDegrees(-45.0f)
	{ }

	virtual void EnforceConstraint(
		int32 Index,
		const TArray<FTransform>& ReferenceCSTransforms,
		const TArray<FIKBoneConstraint*>& Constraints,
		TArray<FTransform>& CSTransforms,
		ACharacter* Character = nullptr
	) override;

	// virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class UPlanarConstraintWrapper : public UIKBoneConstraintWrapper
{ 
	GENERATED_BODY()

public: 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FPlanarRotation Constraint;

	// Subclasses must override this to return the internal constraint struct
	virtual FIKBoneConstraint* GetConstraint() override { return &Constraint; };
};