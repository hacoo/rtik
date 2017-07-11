// Copyright (c) Henry Cooney 2017

/*
* Contains basic IK structures.
*/

#pragma once

#include "ue4ik.h"
#include "AnimNode_SkeletalControlBase.h"
#include "BoneControllers/AnimNode_Fabrik.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IK.generated.h"

#define ENABLE_IK_DEBUG (1 && !(UE_BUILD_SHIPPING || UE_BUILD_TEST))
#define ENABLE_IK_DEBUG_VERBOSE (1 && !(UE_BUILD_SHIPPING || UE_BUILD_TEST))


/*
* A bone used in IK
*/
USTRUCT(BlueprintType)
struct FIKBone
{
	GENERATED_USTRUCT_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FBoneReference BoneRef;

	FCompactPoseBoneIndex BoneIndex;

public:
	
	FIKBone()
		:
		BoneIndex(INDEX_NONE)
	{ }
	

/*
	FIKBone(const FIKBone& Other)
		:
		BoneIndex(Other.BoneIndex)
	{
		UE_LOG(LogIK, Warning, TEXT("hi from the copy constructor"));
		BoneRef.BoneIndex = Other.BoneRef.BoneIndex;
		BoneRef.BoneName = Other.BoneRef.BoneName;
		BoneRef.bUseSkeletonIndex = Other.BoneRef.bUseSkeletonIndex;
		BoneRef.CachedCompactPoseIndex = Other.BoneRef.CachedCompactPoseIndex;
	}

	FIKBone& operator= (const FIKBone& Other)
	{		
		UE_LOG(LogIK, Warning, TEXT("hi from the assignment operator"));
		BoneRef.BoneIndex = Other.BoneRef.BoneIndex;
		BoneRef.BoneName = Other.BoneRef.BoneName;
		BoneRef.bUseSkeletonIndex = Other.BoneRef.bUseSkeletonIndex;
		BoneRef.CachedCompactPoseIndex = Other.BoneRef.CachedCompactPoseIndex;
		BoneIndex = Other.BoneIndex;

		UE_LOG(LogIK, Warning, TEXT("%d"), BoneRef.BoneIndex);
		return *this;
	}
*/

    // Check if this bone is valid, if not, attempt to initialize it. Return whether the bone is (after re-initialization if needed)
	bool InitIfInvalid(const FBoneContainer& RequiredBones)
	{
		if (IsValid(RequiredBones))
		{
			return true;
		}

		Init(RequiredBones);
		bool bIsValid = Init(RequiredBones);
		return bIsValid;
	}

    // Initialize this IK Bone. Must be called before use.
	bool Init(const FBoneContainer& RequiredBones)
	{
		if (BoneRef.Initialize(RequiredBones))
		{
			BoneIndex = BoneRef.GetCompactPoseIndex(RequiredBones);
			return true;
		}
		else
		{
#if ENABLE_IK_DEBUG
			UE_LOG(LogIK, Warning, TEXT("FIKBone::Init -- IK Bone initialization failed for bone: %s"), 
				*BoneRef.BoneName.ToString());
#endif // ENABLE_ANIM_DEBUG
			return false;
		}
	}

	bool IsValid(const FBoneContainer& RequiredBones)
	{
		bool bValid = BoneRef.IsValid(RequiredBones);

#if ENABLE_IK_DEBUG_VERBOSE
		if (!bValid)
		{
			UE_LOG(LogIK, Warning, TEXT("FIKBone::IsValid -- IK Bone %s was invalid"), 
				*BoneRef.BoneName.ToString());
		}
#endif // ENABLE_IK_DEBUG_VERBOSE
		return bValid;
	}


};

/*
* A basic FABRIK IK chain. Requires only a root bone and effector. 
* The effector must be a skeletal child of the root. 
* 
* The InitBoneReferences function must be called by the using animnode before use. 
* This function should initialize bone references, and assign the RootBone and EffectorBone as needed.
*/
USTRUCT(BlueprintType)
struct FFabrikIKChain 
{	
	GENERATED_USTRUCT_BODY()

public:
		
	FFabrikIKChain()
		:
	bInitSuccess(false),
	Precision(0.01f),
	MaxIterations(10)
	{ }

	FAnimNode_Fabrik FabrikSolver;
	FIKBone RootBone;
	FIKBone EffectorBone;	

	// How close the effector tip needs to be before FABRIK solver stops. Decrease for 
    // better results and worse performance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solver")
	float Precision;
	
	// Maximum number of iterations the solver may run for. Increase for better results
    // but possibly worse performance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solver")
	float MaxIterations;
	
	// Checks if this chain is valid; if not, attempts to initialize it and checks again.
    // Returns true if valid or initialization succeeds.
	bool InitIfInvalid(const FBoneContainer& RequiredBones);

	// Initialize all bones used in this chain. Must be called before use.
	bool InitBoneReferences(const FBoneContainer& RequiredBones);

	// Check whether this chain is valid to use. Should be called in the IsValid method of your animnode.
	bool IsValid(const FBoneContainer& RequiredBones);

protected:
	// Subclasses must implement this function so that:
    // - All additional bones are initialized
    // - RootBone and EffectorBone are assigned as needed
	virtual bool InitAndAssignBones(const FBoneContainer& RequiredBones);

	// Subclasses must implement this function so that all additional bones are tested for validity
	virtual bool IsValidInternal(const FBoneContainer& RequiredBones);

	bool bInitSuccess;
};

UCLASS(BlueprintType)
class UIKTestType : public UObject
{
	GENERATED_BODY()

public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone IKBone;	
};
