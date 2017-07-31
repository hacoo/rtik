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
*	How the ROM constraint should behave.
*/
UENUM(BlueprintType)
enum class EIKROMConstraintMode : uint8
{	
	// Constrain both pitch and yaw rotations
	IKROM_Pitch_And_Yaw UMETA(DisplayName = "Constrain Pitch and Yaw"),

	// Constrain pitch rotation; allow no yaw rotation
	IKROM_Pitch_Only UMETA(DisplayName = "Constrain and Allow Only Pitch Rotation"),
	
	// Constrain yaw rotation; allow no pitch rotation
	IKROM_Yaw_Only UMETA(DisplayName = "Constrain and Allow Only Yaw Rotation"),

	// Constrain yaw rotation; allow no pitch rotation
	// IKROM_Twist_Only UMETA(DisplayName = "Constrain and Allow Only Twist Rotation"),

	// Do not constrain rotation
	IKROM_No_Constraint UMETA(DisplayName = "No Constraint")
};


UENUM(BlueprintType)
enum class EIKBoneAxis : uint8
{
	IKBA_X UMETA(DisplayName = "X"),
	IKBA_Y UMETA(DisplayName = "Y"),
	IKBA_Z UMETA(DisplayName = "Z")
};

// Convert an EIKBoneAxis to an EAxis
uint8 IKBoneAxisToAxis(EIKBoneAxis InBoneAxis);

/*
* A bone used in IK.
*
* Range of motion constraints can be specified, but are not necessary unless the bone is being used
* with an IK method that supports them.
*
* Range-of-motion is determined as follows:
* - Pitch and yaw are determined by compaing directions of the bone with the direction of the parent bone. Bone direction is defined as the vector starting at the bone's skeletal parent, and ending at the bone.
* - The pitch / yaw axes of the PARENT bone are used for rotations.
* - Twist (roll) constraints are NOT implemented yet; they are ignored.
* - If a bone does not have a valid parent bone, then the DEfaultForwardDirectionComponentSpace vector is used in place of the parent direction.
*   The rotation axes are simply specified pitch / yaw axes as component space vectors.
* 
* Note that pitch / yaw share a constraint angle for now. This is because it is much simpler and cheaper
* to interect a vector with a circle than an elipsoid. I may change this if needed in the future.
*/
USTRUCT(BlueprintType)
struct UE4IK_API FIKBone
{
	GENERATED_USTRUCT_BODY()
		
public:
	
	FIKBone()
		:
		PitchAxis(EIKBoneAxis::IKBA_Y),
		YawAxis(EIKBoneAxis::IKBA_Z),
		PitchYawROMDegrees(45.0f),
		ConstraintMode(EIKROMConstraintMode::IKROM_No_Constraint),
		BoneIndex(INDEX_NONE)
	{ }
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FBoneReference BoneRef;
		
	// The axis that this bone pitches around. Usually, this points to the side (the Y axis)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EIKBoneAxis PitchAxis;

	// The axis that this bone yaws around. Usually, this points up (the Z axis)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EIKBoneAxis YawAxis;

	// A direction in component space, describing the default 'forward' direction of this bone.
	// If this bone does not have a parent, this direction will be used in place of the parent bone direction.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector DefaultForwardDirectionComponentSpace;
   
	// How far the bone may rotate in the pitch / yaw directions, if these ROM constraints are enforced.	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float PitchYawROMDegrees;

	// How this bone's ROM constraint should be enforced.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EIKROMConstraintMode ConstraintMode;

	FCompactPoseBoneIndex BoneIndex;

public:

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

protected:

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
* A basic IK chain. Doesn't contain any data yet, just an interface for testing validity. 
*
* The InitBoneReferences function must be called by the using animnode before use.
* This function should initialize bone references, and assign the RootBone and EffectorBone as needed.
*/
USTRUCT(BlueprintType)
struct UE4IK_API FIKModChain 
{
	GENERATED_USTRUCT_BODY()
		
public:
		   
	// Checks if this chain is valid; if not, attempts to initialize it and checks again.
    // Returns true if valid or initialization succeeds.
	virtual bool InitIfInvalid(const FBoneContainer& RequiredBones);
	
	// Initialize all bones used in this chain. Must be called before use.
	// Subclasses must override this.
	virtual bool InitBoneReferences(const FBoneContainer& RequiredBones);
	
	// Check whether this chain is valid to use. Should be called in the IsValid method of your animnode.
	// Subclasses must override this.
	virtual bool IsValid(const FBoneContainer& RequiredBones);	
};

/*
* An IK chain with range limits.
*/
USTRUCT(BlueprintType)
struct UE4IK_API FRangeLimitedIKChain : public FIKModChain
{
	
	GENERATED_USTRUCT_BODY()

public:

	FRangeLimitedIKChain()
		:
		bValid(false)
	{ }

	// Bones in the chain, ordered from the effector bone to the root.
	// Each bone must be the skeletal parent of the preceeding bone. 	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TArray<FIKBone> BonesEffectorToRoot;

	// Maps bone indices to their corresponding FIKBone entries in BonesEffectorToRoot
	TArray<size_t> BoneIndexToChainIndexMap;

	FIKBone& operator[](size_t i);

	// access the ith bone, starting from the effector (identical to operator [])
	FIKBone& AccessFromEffector(size_t i);

	// access the ith bone from the root - i.e., accesses bones in reverse order
	FIKBone& AccessFromRoot(size_t i);
	
	size_t Num();

	// Begin FIKModChain interface
	virtual bool InitBoneReferences(const FBoneContainer& RequiredBones) override;
	virtual bool IsValid(const FBoneContainer& RequiredBones) override;
	// End FIKModChain interface

protected:

	bool bValid;

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


UCLASS(BlueprintType, EditInlineNew)
class UE4IK_API URangeLimitedIKChainWrapper : public UIKChainWrapper
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FRangeLimitedIKChain Chain;

	UFUNCTION(BlueprintCallable, Category = IK)
	void Initialize(FRangeLimitedIKChain InChain) 
	{		
		Chain = InChain;
		bInitialized = true;
	}

	virtual bool InitIfInvalid(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
#if ENABLE_IK_DEBUG
			UE_LOG(LogIK, Warning, TEXT("Range limited IK chain wrapper was not initialized -- make sure you call Initialize function in blueprint before use"));
#endif // ENABLE_IK_DEBUG
			return false;
		}
		return Chain.InitIfInvalid(RequiredBones);
	}
	
	// Initialize all bones used in this chain. Must be called before use.
	virtual bool InitBoneReferences(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
#if ENABLE_IK_DEBUG
			UE_LOG(LogIK, Warning, TEXT("Range limited IK chain wrapper was not initialized -- make sure you call Initialize function in blueprint before use"));
#endif // ENABLE_IK_DEBUG
			return false;
		}
		return Chain.InitIfInvalid(RequiredBones);
	}
	
	// Check whether this chain is valid to use. Should be called in the IsValid method of your animnode.
	virtual bool IsValid(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
			return false;
		}
		return Chain.IsValid(RequiredBones);
	}
};

