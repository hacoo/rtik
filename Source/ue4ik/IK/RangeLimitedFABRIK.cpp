// Copyright (c) Henry Cooney 2017

#include "RangeLimitedFABRIK.h"
#include "Utility/DebugDrawUtil.h"

bool FRangeLimitedFABRIK::SolveRangeLimitedFABRIK(
	const TArray<FTransform>& InTransforms,
	const TArray<FIKBoneConstraint*>& Constraints,
	const FVector & EffectorTargetLocation,
	TArray<FTransform>& OutTransforms,
	float MaxRootDragDistance,
	float RootDragStiffness,
	float Precision,
	int32 MaxIterations,
	ACharacter* Character)
{
	OutTransforms.Empty();

	// Number of points in the chain. Number of bones = NumPoints - 1
	int32 NumPoints = InTransforms.Num();

	// Gather bone transforms
	OutTransforms.Reserve(NumPoints);
	for (const FTransform& Transform : InTransforms)
	{
		OutTransforms.Add(Transform);
	}

	if (NumPoints < 2)
	{
		// Need at least one bone to do IK!
		return false;
	}
	
	// Gather bone lengths. BoneLengths contains the length of the bone ENDING at this point,
	// i.e., BoneLengths[i] contains the distance between point i-1 and point i
	float MaximumReach = 0.0f;
	TArray<float> BoneLengths;
	BoneLengths.Reserve(NumPoints);
	BoneLengths.Add(0.0f);

	for (int32 i = 1; i < NumPoints; ++i)
	{
		BoneLengths.Add(FVector::Dist(OutTransforms[i - 1].GetLocation(),
			OutTransforms[i].GetLocation()));
		MaximumReach  += BoneLengths[i];
	}

	bool bBoneLocationUpdated = false;
	float RootToTargetDistSq  = FVector::DistSquared(OutTransforms[0].GetLocation(), EffectorTargetLocation);
	int32 EffectorIndex    = NumPoints - 1;
	
	// Check distance between tip location and effector location
	float Slop = FVector::Dist(OutTransforms[EffectorIndex].GetLocation(), EffectorTargetLocation);
	if (Slop > Precision)
	{
		// Set tip bone at end effector location.
		OutTransforms[EffectorIndex].SetLocation(EffectorTargetLocation);
		
		int32 IterationCount = 0;
		while ((Slop > Precision) && (IterationCount++ < MaxIterations))
		{
			// "Forward Reaching" stage - adjust bones from end effector.
			FABRIKForwardPass(
				InTransforms,
				Constraints,
				BoneLengths,
				OutTransforms,
				Character
			);
			
			// Drag the root if enabled
			DragRoot(
				InTransforms,
				MaxRootDragDistance,
				RootDragStiffness,
				BoneLengths,
				OutTransforms
			);

			// "Backward Reaching" stage - adjust bones from root.
			FABRIKBackwardPass(
				InTransforms,
				Constraints,
				BoneLengths,
				OutTransforms,
				Character
			);

			// Re-check distance between tip location and effector location
			// Since we're keeping tip on top of effector location, check with its parent bone.
			Slop = FMath::Abs(BoneLengths[EffectorIndex] -
				FVector::Dist(OutTransforms[EffectorIndex - 1].GetLocation(), EffectorTargetLocation));
		}
		
		// Place tip bone based on how close we got to target.
		{
			FTransform& ParentLink = OutTransforms[EffectorIndex - 1];
			FTransform& CurrentLink = OutTransforms[EffectorIndex];
			
			CurrentLink.SetLocation(ParentLink.GetLocation() +
				(CurrentLink.GetLocation() - ParentLink.GetLocation()).GetUnsafeNormal() *
				BoneLengths[EffectorIndex]);
		}
		
		bBoneLocationUpdated = true;
	}
	
	// Update bone rotations
	if (bBoneLocationUpdated)
	{
		for (int32 PointIndex = 0; PointIndex < NumPoints - 1; ++PointIndex)
		{
			if (!FMath::IsNearlyZero(BoneLengths[PointIndex + 1]))
			{
				UpdateParentRotation(OutTransforms[PointIndex], InTransforms[PointIndex],
					OutTransforms[PointIndex + 1], InTransforms[PointIndex + 1]);
			}
		}
	}

	return bBoneLocationUpdated;
}

bool FRangeLimitedFABRIK::SolveClosedLoopFABRIK(
	const TArray<FTransform>& InTransforms,
	const TArray<FIKBoneConstraint*>& Constraints,
	const FVector& EffectorTargetLocation,
	TArray<FTransform>& OutTransforms,
	float MaxRootDragDistance,
	float RootDragStiffness,
	float Precision,
	int32 MaxIterations,
	ACharacter* Character
)
{
	OutTransforms.Empty();
		
	// Number of points in the chain. Number of bones = NumPoints - 1
	int32 NumPoints = InTransforms.Num();

	// Gather bone transforms
	OutTransforms.Reserve(NumPoints);
	for (const FTransform& Transform : InTransforms)
	{
		OutTransforms.Add(Transform);
	}

	if (NumPoints < 2)
	{
		// Need at least one bone to do IK!
		return false;
	}



	return true;
};


void FRangeLimitedFABRIK::FABRIKForwardPass(
	const TArray<FTransform>& InTransforms,
	const TArray<FIKBoneConstraint*>& Constraints,
	TArray<float>& BoneLengths,
	TArray<FTransform>& OutTransforms,
	ACharacter* Character
)
{
	int32 NumPoints     = InTransforms.Num();
	int32 EffectorIndex = NumPoints - 1;

	for (int32 PointIndex = EffectorIndex - 1; PointIndex > 0; --PointIndex)
	{
		FTransform& CurrentPoint = OutTransforms[PointIndex];
		FTransform& ChildPoint = OutTransforms[PointIndex + 1];

		if (FMath::IsNearlyZero(BoneLengths[PointIndex + 1]))
		{
			CurrentPoint.SetLocation(ChildPoint.GetLocation());
		}
		else
		{
			CurrentPoint.SetLocation(ChildPoint.GetLocation() +
				(CurrentPoint.GetLocation() - ChildPoint.GetLocation()).GetUnsafeNormal() *
				BoneLengths[PointIndex + 1]);
		}

		// Enforce parent's constraint any time child is moved
		FIKBoneConstraint* CurrentConstraint = Constraints[PointIndex - 1];
		if (CurrentConstraint != nullptr && CurrentConstraint->bEnabled)
		{
			CurrentConstraint->SetupFn(
				PointIndex - 1,
				InTransforms,
				Constraints,
				OutTransforms
			);

			CurrentConstraint->EnforceConstraint(
				PointIndex - 1,
				InTransforms,
				Constraints,
				OutTransforms,
				Character
			);
		}
	}
}
	
void FRangeLimitedFABRIK::FABRIKBackwardPass(
	const TArray<FTransform>& InTransforms,
	const TArray<FIKBoneConstraint*>& Constraints,
	TArray<float>& BoneLengths,
	TArray<FTransform>& OutTransforms,
	ACharacter* Character
	)
{
	int32 NumPoints     = InTransforms.Num();
	int32 EffectorIndex = NumPoints - 1;

	for (int32 PointIndex = 1; PointIndex < EffectorIndex; PointIndex++)
	{
		FTransform& ParentPoint = OutTransforms[PointIndex - 1];
		FTransform& CurrentPoint = OutTransforms[PointIndex];
		
		if (FMath::IsNearlyZero(BoneLengths[PointIndex]))
		{
			CurrentPoint.SetLocation(ParentPoint.GetLocation());
		}
		else
		{
			CurrentPoint.SetLocation(ParentPoint.GetLocation() +
				(CurrentPoint.GetLocation() - ParentPoint.GetLocation()).GetUnsafeNormal() *
				BoneLengths[PointIndex]);
		}
		
		// Enforce parent's constraint any time child is moved
		FIKBoneConstraint* CurrentConstraint = Constraints[PointIndex - 1];
		if (CurrentConstraint != nullptr && CurrentConstraint->bEnabled)
		{
			CurrentConstraint->SetupFn(
				PointIndex - 1,
				InTransforms,
				Constraints,
				OutTransforms
			);
			
			CurrentConstraint->EnforceConstraint(
				PointIndex - 1,
				InTransforms,
				Constraints,
				OutTransforms,
				Character
			);
		}
	}
}

void FRangeLimitedFABRIK::DragRoot(
	const TArray<FTransform>& InTransforms,
	float MaxRootDragDistance,
	float RootDragStiffness,
	TArray<float>& BoneLengths,
	TArray<FTransform>& OutTransforms
)
{
	if (MaxRootDragDistance < SMALL_NUMBER)
	{
		return;
	}
		
	FTransform& ChildPoint = OutTransforms[1];
	FVector RootTarget;

	if (FMath::IsNearlyZero(BoneLengths[1]))
	{
		RootTarget = ChildPoint.GetLocation();
	}
	else
	{
		RootTarget = ChildPoint.GetLocation() +
			(OutTransforms[0].GetLocation() - ChildPoint.GetLocation()).GetUnsafeNormal() *
			BoneLengths[1];
	}
		
	// Root drag stiffness pulls the root back if enabled
	FVector RootDisplacement = RootTarget - InTransforms[0].GetLocation();
	if (RootDragStiffness > KINDA_SMALL_NUMBER)
	{
		RootDisplacement /= RootDragStiffness;
	}
	
	// limit root displacement to drag length
	FVector RootLimitedDisplacement = RootDisplacement.GetClampedToMaxSize(MaxRootDragDistance);
	OutTransforms[0].SetLocation(InTransforms[0].GetLocation() + RootLimitedDisplacement);
}


void FRangeLimitedFABRIK::UpdateParentRotation(
	FTransform& NewParentTransform, 
	const FTransform& OldParentTransform,
	const FTransform& NewChildTransform,
	const FTransform& OldChildTransform)
{
	FVector OldDir = (OldChildTransform.GetLocation() - OldParentTransform.GetLocation()).GetUnsafeNormal();
	FVector NewDir = (NewChildTransform.GetLocation() - NewParentTransform.GetLocation()).GetUnsafeNormal();
	
	// Calculate axis of rotation from pre-translation vector to post-translation vector
	FVector RotationAxis = FVector::CrossProduct(OldDir, NewDir).GetSafeNormal();
	float RotationAngle = FMath::Acos(FVector::DotProduct(OldDir, NewDir));
	FQuat DeltaRotation = FQuat(RotationAxis, RotationAngle);
	// We're going to multiply it, in order to not have to re-normalize the final quaternion, it has to be a unit quaternion.
	checkSlow(DeltaRotation.IsNormalized());
	
	// Calculate absolute rotation and set it
	NewParentTransform.SetRotation(DeltaRotation * OldParentTransform.GetRotation());
	NewParentTransform.NormalizeRotation();
}
