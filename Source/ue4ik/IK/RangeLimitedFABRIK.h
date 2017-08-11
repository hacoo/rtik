// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "IK.h"

/*
	Range-limited FABRIK solvers and related functions. Does not need to be in the context of a Skeleton; 
	this solver is designed to work with generic transforms. Make sure all transforms are in the same space, though!

	See www.andreasaristidou.com/FABRIK.html for details of the FABRIK algorithm.
*/

/* 
* A three-point closed loop, containing two noisy effectors.
* This is a very specific type of IK 'chain', used for the torso upper and lower body triangles.
*/
struct UE4IK_API FNoisyThreePointClosedLoop
{
public:

	FNoisyThreePointClosedLoop(const FTransform& InEffectorATransform, 
		const FTransform& InEffectorBTransform,
		const FTransform& InRootTransform,
		float InTargetRootADistance,
		float InTargetRootBDistance,
		float InTargetABDistance)
		:
		EffectorATransform(InEffectorATransform),
		EffectorBTransform(InEffectorBTransform),
		RootTransform(InRootTransform),
		TargetRootADistance(InTargetRootADistance),
		TargetRootBDistance(InTargetRootBDistance),
		TargetABDistance(InTargetABDistance)
	{ }
		
	FNoisyThreePointClosedLoop()
	{ }

	// The first noisy effector
	FTransform EffectorATransform;

	// The second noisy effector
	FTransform EffectorBTransform;

	// The root transform. Not IKed onto a target location, but may be constrained not to drag too far from
	// its starting position	
	FTransform RootTransform;

	// The desired distance between A and the root	
	float TargetRootADistance; 

	// The desired distance between B and the root
	float TargetRootBDistance;

	// The desired distance between A and B
	float TargetABDistance;
};

struct UE4IK_API FRangeLimitedFABRIK
{
public:
	
	/*
	* Uses the FABRIK algorithm to solve the IK problem on a chain of rigidly-connected points.	
	*  
	* InTransforms is a list of transforms representing the starting position of each point. The 0th element
    * of this array is the ROOT. The last element is the EFFECTOR (sometimes called the 'tip' in code).
	*
	* The root point represents the start of the chain; its displacement from its starting position is limited.
	* The effector represents the end of the chain, FABRIK will attempt to move this to EffectorTargetLocation,
	* while maintaining inter-point distances.
	*
	* Each chain point, except the effector, has a CHILD - this is simply the next point in the chain. Correspondingly,
	* each point except the root has a PARENT, which is the previous point. The displacement between a parent and its child 
	* is loosely referred to as a BONE.	
	* 
	* FABRIK is an iterative algorithm. It will adjust each point in the chain in a forward-and-backward manner,
	* until either the distance between the effector and EffectorTargetLocation is less than Precision, or MaxIterations
	* iterations (up and down the chain) have been performed.
	*
	* In principal, FABRIK pays attention only to the location of each chain point, not their rotations. However, 
	* this is impractical when working with skeletal bones. Once the final locations of each chain point are 
	* determined, the rotation of each transform is updated as follows:
	* 
	* - Let P be a chain point which is not effector, and let C be its child, before FABRIK
	* - Let P' and C' be the corresponding chain points after FABRIK is applied
	* - Let Q be the shortest rotation from (C - P) to (C' - P')
	* - Add the rotation Q to the rotation of P'	
	*
	* In other words, the rotation of each point transform is updated with the smallest rotation to make it 
	* 'point toward' the newly adjusted child point.	
	*
	* The effector's rotation is NOT updated (unlike in the UE4 FABRIK implementation), you will need to do this
	* yourself after running FABRIK.
	*
	* Parameter descriptions follow:
	* 	
	* @param InTransforsm - The starting transforms of each chain point. Not modified. Must contain at least 2 transforms.
	* @param Constraints - Constraints corresponding to each chain point; entries should be set to nullptr for points that don't need a constraint.
	*   these may modify chain transforms arbitrarily and are enforced each time a point is moved.
	*	Strong constraints will degrade the results of FABRIK, it's up to you to figure out what works.
	* @param EffectorTargetLocation - Where you want the effector to go. FABRIK will attempt to move the effector as close
	*   as possible to this point.
	* @param OutTransforms - The updated transforms o each chain point after FABRIK runs. Shoudl be an empty array. Will be emptied and filled with new transforms.  
	* @param MaxRootDragDistance - How far the root may move from its original position. Set to 0 for no movement.
	* @param RootDragStiffness - How much the root will resist being moved from the original position. 1.0 means no resistance; 
	*   increase for more resistance. Settings less than 1.0 will make it move more.
	* @param Precision - Iteration will terminate when the effector is within this distance from the target.
	*  Decrease for possibly better results but possibly worse performance.
	* @param MaxIterations - The maximum number of iterations to run. Increase for possibly better results but 
	*  possibly worse performance.
	* @param Character - Character pointer whcih may be used for debug drawing. May safely be set to nullptr or ignored.
	* @return - True if any transforms in OutTransforms were updated; otherwise, false. If false, the contents of OutTransforms is identical to InTransforms.
	*/
	static bool SolveRangeLimitedFABRIK(
		const TArray<FTransform>& InTransforms,
		const TArray<FIKBoneConstraint*>& Constraints,
		const FVector& EffectorTargetLocation,
		TArray<FTransform>& OutTransforms,
		float MaxRootDragDistance = 0.0f,
		float RootDragStiffness = 1.0f,
		float Precision = 0.01f,
		int32 MaxIterations = 20,
		ACharacter* Character = nullptr
	);

	/*
	* Solves FABRIK on a CLOSED LOOP, that is, a chain where the effector is assumed to be connected to the root.
	* 	
	* I'm not sure how well constraints will work with this solver, but they are nonetheless supported for the sake of
	* keeping a consistent interface. You're welcome to try them out but results may be bad.	
	*
	* Note that you will probably HAVE to use root dragging if you want this solver to work!
	*/
	static bool SolveClosedLoopFABRIK(
		const TArray<FTransform>& InTransforms,
		const TArray<FIKBoneConstraint*>& Constraints,
		const FVector& EffectorTargetLocation,
		TArray<FTransform>& OutTransforms,
		float MaxRootDragDistance = 10.0f,
		float RootDragStiffness = 1.0f,
		float Precision = 0.01f,
		int32 MaxIterations = 20,
		ACharacter* Character = nullptr
	);

	/*
	* Runs closed-loop FABRIK multiple times, attempting to move both 'noisy effectors' to their targets.
	* See www.andreasaristidou.com/publications/papers/Extending_FABRIK_with_Model_Cοnstraints.pdf
	*/
	static bool SolveNoisyThreePoint(
		const FNoisyThreePointClosedLoop& InClosedLoop,
		const FTransform& EffectorATarget,
		const FTransform& EffectorBTarget,
		FNoisyThreePointClosedLoop& OutClosedLoop,
		float MaxRootDragDistance = 0.0f,
		float RootDragStiffness = 1.0f,
		float Precision = 0.01f,
		int32 MaxIterations = 20,
		ACharacter* Character = nullptr		
	);
	
protected:

	static void UpdateParentRotation(
		FTransform& NewParentTransform,
		const FTransform& OldParentTransform,
		const FTransform& NewChildTransform,
		const FTransform& OldChildTransform
	);

	// Iterate from effector to root
	static void FABRIKForwardPass(
		const TArray<FTransform>& InTransforms,
		const TArray<FIKBoneConstraint*>& Constraints,
		const TArray<float>& BoneLengths,
		TArray<FTransform>& OutTransforms,
		ACharacter* Character = nullptr 
	);
	
	// Iterate from root to effector
	static void FABRIKBackwardPass(
		const TArray<FTransform>& InTransforms,
		const TArray<FIKBoneConstraint*>& Constraints,
		const TArray<float>& BoneLengths,
		TArray<FTransform>& OutTransforms,
		ACharacter* Character = nullptr
	);

	// The core FABRIK method. Projects PointToMove onto the vector between itself and MaintainDistancePoint, 
	// such that the distance between them is BoneLength. This enforces the core FABRIK constraint, that inter-point
	// distances don't change. Thus, PointToMove is 'dragged' toward MaintainDistancePoint and the original interpoint
	// distance is maintained.
	static FORCEINLINE void DragPoint(
		const FTransform& MaintainDistancePoint,
		float BoneLength,
		FTransform& PointToMove
	);

	// Drags PointToDrag relative to MaintainDistancePoint; that is, PointToDrag is moved so that it attempts
	// to maintain the distance BoneLength between itself and MaintainDistancePoint. However, PointToDrag is 'tethered'
	// to TetherPoint; it cannot ever be dragged father than MaxDragDistance from TetherPoint. Additionally, the 
	// tether acts like a spring, instantaneously pulling MaintainDistancePoint back toward TetherPoint by an amount dictated by DragStiffnes;
	// specifically, the required displacement (before clamping to MaxDragDisplacement) is divided by DragStiffness.
	// 
	// Basically, this is like EnforcePointDistance, but with the additional stronger constraint that PointToDrag
	// can never be moved father than MaxDragDistance from StartingTransform.
	static void DragPointTethered(
		const FTransform& TetherPoint,
		const FTransform& MaintainDistancePoint,
		float BoneLength,
		float MaxDragDistance,
		float DragStiffness,
		FTransform& PointToDrag
	);

	// Compute bone lengths and store in BoneLengths. BoneLengths will be emptied and refilled.
	// Each entry contains the length of bone ending at point i, i.e., OutBoneLengths[i] contains the starting distance 
	// between point i and point i-1.
	// Returns the maximum reach.
	static float ComputeBoneLengths(
		const TArray<FTransform>& InTransforms,
		TArray<float>& OutBoneLengths
	);
};