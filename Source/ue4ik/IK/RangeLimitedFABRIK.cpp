// Copyright (c) Henry Cooney 2017

#include "RangeLimitedFABRIK.h"

bool FRangeLimitedFABRIK::SolveRangeLimitedFABRIK(
	const TArray<FTransform>& InCSTransforms,
	const TArray<UIKBoneConstraint*>& Constraints,
	const FVector & EffectorTargetLocationCS,
	TArray<FTransform>& OutCSTransforms,
	float Precision,
	int32 MaxIterations,
	ACharacter* Character)
{
	OutCSTransforms.Empty();

	int32 NumBones = InCSTransforms.Num();

	if (NumBones < 1)
	{
		return false;
	}

	// Gather bone transforms
	OutCSTransforms.Reserve(NumBones);
	for (const FTransform& Transform : InCSTransforms)
	{
		OutCSTransforms.Add(Transform);
	}
	
	// Gather bone lengths
	float MaximumReach = 0.0f;
	TArray<float> BoneLengths;
	BoneLengths.Reserve(NumBones);
	BoneLengths.Add(0.0f);

	for (int32 i = 1; i < NumBones; ++i)
	{
		BoneLengths.Add(FVector::Dist(OutCSTransforms[i - 1].GetLocation(),
			OutCSTransforms[i].GetLocation()));
		MaximumReach  += BoneLengths[i];
	}

	bool bBoneLocationUpdated = false;

	float RootToTargetDistSq = FVector::DistSquared(OutCSTransforms[0].GetLocation(), EffectorTargetLocationCS);

	/*
	// FABRIK algorithm - bone translation calculation
	// If the effector is further away than the distance from root to tip, simply move all bones in a line from root to effector location
	if (RootToTargetDistSq > FMath::Square(MaximumReach))
	{
		for (int32 LinkIndex = 1; LinkIndex < NumBones; LinkIndex++)
		{
			FTransform& ParentLink = OutCSTransforms[LinkIndex - 1];
			FTransform& CurrentLink = OutCSTransforms[LinkIndex];
			CurrentLink.SetLocation(ParentLink.GetLocation() +
				(EffectorTargetLocationCS - ParentLink.GetLocation()).GetUnsafeNormal() *
				BoneLengths[LinkIndex]);
		}
		bBoneLocationUpdated = true;
	}
	else // Effector is within reach, calculate bone translations to position tip at effector location
	{
	*/
	int32 TipBoneLinkIndex = NumBones - 1;
	
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
					BoneLengths[LinkIndex]);
				
				// Enforce parent's constraint any time child is moved
				UIKBoneConstraint* CurrentConstraint = Constraints[LinkIndex - 1];
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
			
			// "Backward Reaching" stage - adjust bones from root.
			for (int32 LinkIndex = 1; LinkIndex < TipBoneLinkIndex; LinkIndex++)
			{
				FTransform& ParentLink = OutCSTransforms[LinkIndex - 1];
				FTransform& CurrentLink = OutCSTransforms[LinkIndex];
				
				CurrentLink.SetLocation(ParentLink.GetLocation() +
					(CurrentLink.GetLocation() - ParentLink.GetLocation()).GetUnsafeNormal() *
					BoneLengths[LinkIndex]);
				
				// Enforce parent's constraint any time child is moved
				UIKBoneConstraint* CurrentConstraint = Constraints[LinkIndex - 1];
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
			
			//UpdateParentRotation(ParentLink, IKChain->Chain[TipBoneLinkIndex - 1],
			//CurrentLink, IKChain->Chain[TipBoneLinkIndex],
			//Output.Pose);
		}
		
		bBoneLocationUpdated = true;
	}
//}
	
	// Update bone rotations
	if (bBoneLocationUpdated)
	{
		for (int32 LinkIndex = 0; LinkIndex < NumBones - 1; ++LinkIndex)
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
