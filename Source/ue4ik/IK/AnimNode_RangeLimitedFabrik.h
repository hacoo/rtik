// Copyright(c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_RangeLimitedFabrik.generated.h"


/*
	Range-limited FABRIK solver. Based on FABRIK UE4 FABRIK solver (see AnimNode_Fabrik.h).

	Bones may be given a range-of-motion (ROM) constraint, based either on the preceeding bone's direction,
	or a direction in component space. 

	The following constraint modes are supported:

	- Pitch and Yaw: a 'circular' constraint, with the same angular constraint in pitch and yaw directions.
	Twist is not constrained.
	- Yaw only: the joint may only yaw, within constraint angle.
	- Pitch only: the joint may only pitch, within constraint angle.
	- Twist only: the joint may only twist, within constraint angle	
	- No constraint: The joint is not ROM-constrained.
*/

struct FRangeLimitedFABRIKChainLink
{
public:
	FVector Position;

	float Length;

	FCompactPoseBoneIndex BoneIndex;

	int32 TransformIndex;

	TArray<int32> ChildZeroLengthTransformIndices;

	FRangeLimitedFABRIKChainLink()
		: Position(FVector::ZeroVector),
		Length(0.f),
		BoneIndex(INDEX_NONE),
		TransformIndex(INDEX_NONE)
	{ }

	FRangeLimitedFABRIKChainLink(const FVector& InPosition, float InLength,
		const FCompactPoseBoneIndex& InBoneIndex, const int32& InTransformIndex)
		: Position(InPosition),
		Length(InLength),
		BoneIndex(InBoneIndex),
		TransformIndex(InTransformIndex)
	{ }
};

USTRUCT()
struct UE4IK_API FAnimNode_RangeLimitedFabrik : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()

	/** Coordinates for target location of tip bone - if EffectorLocationSpace is bone, this is the offset from Target Bone to use as target location*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EndEffector, meta = (PinShownByDefault))
	FTransform EffectorTransform;

	/** Reference frame of Effector Transform. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EndEffector)
	TEnumAsByte<enum EBoneControlSpace> EffectorTransformSpace;

	/** If EffectorTransformSpace is a bone, this is the bone to use. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EndEffector)
	FBoneReference EffectorTransformBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EndEffector)
	TEnumAsByte<enum EBoneRotationSource> EffectorRotationSource;

	/** Name of tip bone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	FBoneReference TipBone;

	/** Name of the root bone*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	FBoneReference RootBone;

	/** Tolerance for final tip location delta from EffectorLocation*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	float Precision;

	/** Maximum number of iterations allowed, to control performance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	int32 MaxIterations;

	/** Toggle drawing of axes to debug joint rotation*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	bool bEnableDebugDraw;

public:
	FAnimNode_RangeLimitedFabrik();

	// FAnimNode_Base interface
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface

	// FAnimNode_SkeletalControlBase interface
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

private:
	// FAnimNode_SkeletalControlBase interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

	// Convenience function to get current (pre-translation iteration) component space location of bone by bone index
	FVector GetCurrentLocation(FCSPose<FCompactPose>& MeshBases, const FCompactPoseBoneIndex& BoneIndex);

#if WITH_EDITOR
	// Cached CS location when in editor for debug drawing
	FTransform CachedEffectorCSTransform;
#endif
};
