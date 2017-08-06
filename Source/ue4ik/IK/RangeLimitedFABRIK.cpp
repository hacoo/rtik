// Copyright (c) Henry Cooney 2017

#include "RangeLimitedFABRIK.h"
#include "Utility/DebugDrawUtil.h"

bool FRangeLimitedFABRIK::SolveRangeLimitedFABRIK(
	const TArray<FTransform>& InCSTransforms,
	const TArray<FIKBoneConstraint*>& Constraints,
	const FVector & EffectorTargetLocationCS,
	TArray<FTransform>& OutCSTransforms,
	float MaxRootDragDistance,
	float RootDragStiffness,
	float Precision,
	int32 MaxIterations,
	ACharacter* Character)
{
	OutCSTransforms.Empty();

	// Number of points in the chain. Number of bones = NumPoints - 1
	int32 NumPoints = InCSTransforms.Num();

	if (NumPoints < 2)
	{
		// Need at least one bone to do IK!
		return false;
	}

	// Gather bone transforms
	OutCSTransforms.Reserve(NumPoints);
	for (const FTransform& Transform : InCSTransforms)
	{
		OutCSTransforms.Add(Transform);
	}
	
	// Gather bone lengths
	float MaximumReach = 0.0f;
	TArray<float> BoneLengths;
	BoneLengths.Reserve(NumPoints);
	BoneLengths.Add(0.0f);

	for (int32 i = 1; i < NumPoints; ++i)
	{
		BoneLengths.Add(FVector::Dist(OutCSTransforms[i - 1].GetLocation(),
			OutCSTransforms[i].GetLocation()));
		MaximumReach  += BoneLengths[i];
	}

	bool bBoneLocationUpdated = false;

	float RootToTargetDistSq = FVector::DistSquared(OutCSTransforms[0].GetLocation(), EffectorTargetLocationCS);

	int32 TipBoneLinkIndex = NumPoints - 1;
	
	// Check distance between tip location and effector location
	float Slop = FVector::Dist(OutCSTransforms[TipBoneLinkIndex].GetLocation(), EffectorTargetLocationCS);
	if (Slop > Precision)
	{
		// Set tip bone at end effector location.
		OutCSTransforms[TipBoneLinkIndex].SetLocation(EffectorTargetLocationCS);
		
		int32 IterationCount = 0;
		while ((Slop > Precision) && (IterationCount++ < MaxIterations))
		{
			// "Forward Reaching" stage - adjust bones from end effector.
			for (int32 LinkIndex = TipBoneLinkIndex - 1; LinkIndex > 0; LinkIndex--)
			{
				FTransform& CurrentLink = OutCSTransforms[LinkIndex];
				FTransform& ChildLink = OutCSTransforms[LinkIndex + 1];
				
				CurrentLink.SetLocation(ChildLink.GetLocation() +
					(CurrentLink.GetLocation() - ChildLink.GetLocation()).GetUnsafeNormal() *
					BoneLengths[LinkIndex + 1]);
				
				// Enforce parent's constraint any time child is moved
				FIKBoneConstraint* CurrentConstraint = Constraints[LinkIndex - 1];
				if (CurrentConstraint != nullptr && CurrentConstraint->bEnabled)
				{
					CurrentConstraint->SetupFn(
						LinkIndex - 1,
						InCSTransforms,
						Constraints,
						OutCSTransforms
					);
					
					CurrentConstraint->EnforceConstraint(
						LinkIndex - 1,
						InCSTransforms,
						Constraints,
						OutCSTransforms,
						Character
					);
				}
			}
			
			// Drag the root if enabled
			if (MaxRootDragDistance > KINDA_SMALL_NUMBER)
			{
				FTransform& ChildLink = OutCSTransforms[1];

				FVector RootTarget = ChildLink.GetLocation() +
					(OutCSTransforms[0].GetLocation() - ChildLink.GetLocation()).GetUnsafeNormal() *
					BoneLengths[1];

				// Root drag stiffness pulls the root back if enabled
				FVector RootDisplacement = RootTarget - InCSTransforms[0].GetLocation();
				if (RootDragStiffness > KINDA_SMALL_NUMBER)
				{
					UE_LOG(LogIK, Warning, TEXT("%f"), RootDisplacement.Size());
					RootDisplacement /= RootDragStiffness;
					UE_LOG(LogIK, Warning, TEXT("%f"), RootDisplacement.Size());					
				}

				// limit root displacement to drag length
				FVector RootLimitedDisplacement = RootDisplacement.GetClampedToMaxSize(MaxRootDragDistance);
				OutCSTransforms[0].SetLocation(InCSTransforms[0].GetLocation() + RootLimitedDisplacement);
			}


			// "Backward Reaching" stage - adjust bones from root.
			for (int32 LinkIndex = 1; LinkIndex < TipBoneLinkIndex; LinkIndex++)
			{
				FTransform& ParentLink = OutCSTransforms[LinkIndex - 1];
				FTransform& CurrentLink = OutCSTransforms[LinkIndex];
				
				CurrentLink.SetLocation(ParentLink.GetLocation() +
					(CurrentLink.GetLocation() - ParentLink.GetLocation()).GetUnsafeNormal() *
					BoneLengths[LinkIndex]);
				
				// Enforce parent's constraint any time child is moved
				FIKBoneConstraint* CurrentConstraint = Constraints[LinkIndex - 1];
				if (CurrentConstraint != nullptr && CurrentConstraint->bEnabled)
				{
					CurrentConstraint->SetupFn(
						LinkIndex - 1,
						InCSTransforms,
						Constraints,
						OutCSTransforms
					);
					
					CurrentConstraint->EnforceConstraint(
						LinkIndex - 1,
						InCSTransforms,
						Constraints,
						OutCSTransforms,
						Character
					);
				}
			}

			// Re-check distance between tip location and effector location
			// Since we're keeping tip on top of effector location, check with its parent bone.
			Slop = FMath::Abs(BoneLengths[TipBoneLinkIndex] -
				FVector::Dist(OutCSTransforms[TipBoneLinkIndex - 1].GetLocation(), EffectorTargetLocationCS));
		}
		
		// Place tip bone based on how close we got to target.
		{
			FTransform& ParentLink = OutCSTransforms[TipBoneLinkIndex - 1];
			FTransform& CurrentLink = OutCSTransforms[TipBoneLinkIndex];
			
			CurrentLink.SetLocation(ParentLink.GetLocation() +
				(CurrentLink.GetLocation() - ParentLink.GetLocation()).GetUnsafeNormal() *
				BoneLengths[TipBoneLinkIndex]);
		}
		
		bBoneLocationUpdated = true;
	}
	
	// Update bone rotations
	if (bBoneLocationUpdated)
	{
		for (int32 LinkIndex = 0; LinkIndex < NumPoints - 1; ++LinkIndex)
		{
			UpdateParentRotation(OutCSTransforms[LinkIndex], InCSTransforms[LinkIndex],
				OutCSTransforms[LinkIndex + 1], InCSTransforms[LinkIndex + 1]);
		}
	}

	return bBoneLocationUpdated;
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
