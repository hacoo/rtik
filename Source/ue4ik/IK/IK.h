// Copyright (c) Henry Cooney 2017

/*
* Contains basic IK structures.
*/

#pragma once

#include "ue4ik.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_SkeletalControlBase.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IK.generated.h"

#define ENABLE_IK_DEBUG (1 && !(UE_BUILD_SHIPPING || UE_BUILD_TEST))
#define ENABLE_IK_DEBUG_VERBOSE (1 && !(UE_BUILD_SHIPPING || UE_BUILD_TEST))


/*
* Specifies what IK should do if the target is unreachable
*/
UENUM(BlueprintType)
enum class EIKUnreachableRule : uint8
{
	// Abort IK, return to pre-IK pose
	IK_Abort        UMETA(DisplayName = "Abort IK"),

	// Reach as far toward the target as possible without moving the root bone
	IK_Reach        UMETA(DisplayName = "Reach for Target"),
	
	// Drag the root bone toward the target so it can be reached (caution, this is likely to give weird results)
	IK_DragRoot     UMETA(DisplayName = "Drag Chain Root")
};

/*
* A bone used in IK
*/
USTRUCT(BlueprintType)
struct UE4IK_API FIKBone
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
* Allows bones to be passed around in BP
*/
UCLASS(BlueprintType, EditInlineNew)
class UE4IK_API UIKBoneWrapper : public UObject
{
	
	GENERATED_BODY()

public: 

	UIKBoneWrapper(const FObjectInitializer& ObjectInitializer)
		: 
		Super(ObjectInitializer),
		bInitialized(false)
	{ }

	UFUNCTION(BlueprintCallable, Category = IK)
	void Initialize(FIKBone InBone)
	{
		Bone = InBone;
		bInitialized = true;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FIKBone Bone;
	
	bool InitIfInvalid(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
#if ENABLE_IK_DEBUG
			UE_LOG(LogIK, Warning, TEXT("IK Bone Wrapper was not initialized -- you must call Initialize in blueprint before use"));
#endif // ENABLE_IK_DEBUG
			return false;
		}

		return Bone.InitIfInvalid(RequiredBones);
	}

	bool Init(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
#if ENABLE_IK_DEBUG
			UE_LOG(LogIK, Warning, TEXT("IK Bone Wrapper was not initialized -- you must call Initialize in blueprint before use"));
#endif // ENABLE_IK_DEBUG
			return false;
		}

		return Bone.Init(RequiredBones);
	}


	bool IsValid(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
			return false;
		}

		return Bone.IsValid(RequiredBones);
	}
	
protected:

	bool bInitialized;
};


/*
* A basic IK chain. Requires only a root bone and effector.
* The effector must be a skeletal child of the root.
*
* The InitBoneReferences function must be called by the using animnode before use.
* This function should initialize bone references, and assign the RootBone and EffectorBone as needed.
*/
USTRUCT(BlueprintType)
struct UE4IK_API FIKModChain 
{
	GENERATED_USTRUCT_BODY()
		
public:
	
	FIKModChain()
		:
		bInitSuccess(false)
	{ }
	
	FIKBone RootBone;
	FIKBone EffectorBone;
   
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

/*
* Wraps an IK chain so it can be passed around in BPs
*/
UCLASS(BlueprintType, EditInlineNew)
class UE4IK_API UIKChainWrapper : public UObject
{
	GENERATED_BODY()

public: 

	UIKChainWrapper(const FObjectInitializer& ObjectInitializer)
		: 
		Super(ObjectInitializer),
		bInitialized(false)
	{ }

	// Subclasses should implement an Initialize method that copies incoming chain
    // into internal struct

	// Checks if this chain is valid; if not, attempts to initialize it and checks again.
    // Returns true if valid or initialization succeeds.
	virtual bool InitIfInvalid(const FBoneContainer& RequiredBones) 
	{
		return false;
	}
	
	// Initialize all bones used in this chain. Must be called before use.
	virtual bool InitBoneReferences(const FBoneContainer& RequiredBones)
	{
		return false;
	}
	
	// Check whether this chain is valid to use. Should be called in the IsValid method of your animnode.
	virtual bool IsValid(const FBoneContainer& RequiredBones)
	{
		return false;
	}
	
protected:
	bool bInitialized;
};