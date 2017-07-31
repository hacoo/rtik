// Copyright(c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "IK/IK.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_RangeLimitedFabrik.generated.h"


/*
	Range-limited FABRIK solver. Based on FABRIK UE4 FABRIK solver (see AnimNode_Fabrik.h), but will
	additionally enforce range-of-motion constraints.

	See IK.h for a description of ROM constraints.
*/

 // Internal structure for evaluating the IK chain
struct FRangeLimitedFABRIKChainLink
{
public:

	float Length;

	FCompactPoseBoneIndex BoneIndex;

	FTransform BoneCSTransform;

	TArray<int32> ChildZeroLengthTransformIndices;

	// The corresponding IK Bone. 
	FIKBone IKBone;

	FRangeLimitedFABRIKChainLink()
		: Length(0.f),
		BoneIndex(INDEX_NONE),
		BoneCSTransform(FTransform::Identity)
	{ }

	FRangeLimitedFABRIKChainLink(float InLength, const FCompactPoseBoneIndex& InBoneIndex,
		const FTransform& InTransform)
		: Length(InLength),
		BoneIndex(InBoneIndex),
		BoneCSTransform(InTransform)
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

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones)
	//URangeLimitedIKChainWrapper* IKChain;

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


protected:
	// FAnimNode_SkeletalControlBase interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

	// Convenience function to get current (pre-translation iteration) component space location of bone by bone index
	FVector GetCurrentLocation(FCSPose<FCompactPose>& MeshBases, const FCompactPoseBoneIndex& BoneIndex);

	void EnforceROMConstraint(FCSPose<FCompactPose>& Pose, TArray<FRangeLimitedFABRIKChainLink>& Chain,
		FIKBone& ChildBone, int32 ChildIndex);

	void UpdateParentRotation(FRangeLimitedFABRIKChainLink& ParentLink,
		const FRangeLimitedFABRIKChainLink& ChildLink, FCSPose<FCompactPose>& Pose);

#if WITH_EDITOR
	// Cached CS location when in editor for debug drawing
	FTransform CachedEffectorCSTransform;
#endif
};
