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
#define ENABLE_IK_DEBUG_VERBOSE (0 && !(UE_BUILD_SHIPPING || UE_BUILD_TEST))


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
	IKBA_Z UMETA(DisplayName = "Z"),
	IKBA_XNeg UMETA(DisplayName = "X Negative"),
	IKBA_YNeg UMETA(DisplayName = "Y Negative"),
	IKBA_ZNeg UMETA(DisplayName = "Z Negative")
};

// IK utility functions
struct FIKUtil
{
public:

	// Convert an EIKBoneAxis to a unit vector.
	static FVector IKBoneAxisToVector(EIKBoneAxis InBoneAxis);

	// Get the specified axis of the skeletal mesh component's component transform
	static FVector GetSkeletalMeshComponentAxis(const USkeletalMeshComponent& SkelComp, EIKBoneAxis InBoneAxis);
};



/*
* A range-of-motion constraint on a bone used in IK.
* 
* ROM constraints have access to the entire bone chain, before and after IK, and
* may modify and and all transforms in the chain. 
* 
* The base constraint type does nothing.
*/

USTRUCT(BlueprintType)
struct UE4IK_API FIKBoneConstraint 
{
	
	GENERATED_USTRUCT_BODY()

public:

	// Constraint should only be enforced if this is set to true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableDebugDraw;

public:

	FIKBoneConstraint()
		:
		bEnabled(true),
		bEnableDebugDraw(false)
	{ }


	// Enforces the constraint. Will modify OutCSTransforms if needed.
	// @param Index - The index of this constraint in Constraints; should correspond to the same bone in in InCSTransforms and OutCSTransforms
	// @param ReferenceCSTransforms - Array of bone transforms before skeletal controls (e.g., IK) are applied. Not necessarily in the reference pose (although they might be, depending on your needs)
	// @param Constraints - Array of constraints for each bone (including this one, at index Index)	
	// @param CSTransforms - Array of transforms as skeletal controls (e.g., IK) are being applied; this array will be modified in place
	// @param Character - Optional owning Character; may to left to nullptr, but is required for debug drawing.
	virtual void EnforceConstraint(
		int32 Index,
		const TArray<FTransform>& ReferenceCSTransforms,
		const TArray<FIKBoneConstraint*>& Constraints,
		TArray<FTransform>& CSTransforms,
		ACharacter* Character = nullptr
	) { }

	// Optional lambda to evaluate before the constraint is enforced. It can set up examine the chain and set 
	// things up appropriately. Returns a bool; the constraint should only be enforced if it returns true.
	TFunction<void(
		int32 Index,
		const TArray<FTransform>& ReferenceCSTransforms,
		const TArray<FIKBoneConstraint*>& Constraints,
		TArray<FTransform>& CSTransforms
		)> SetupFn = [](
			int32 Index,
			const TArray<FTransform>& ReferenceCSTransforms,
			const TArray<FIKBoneConstraint*>& Constraints,
			TArray<FTransform>& CSTransforms
			) {};
};

/*
 * Wrapper class allows these to be set 'polymorphically' during property-window setup
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, abstract)
class UIKBoneConstraintWrapper : public UObject
{ 
	GENERATED_BODY()

public:

	// Subclasses must override this to return the internal constraint struct
	virtual FIKBoneConstraint* GetConstraint() { return nullptr; }
};

/*
* A bone used in IK.
*
* Range of motion constraints can be specified, but are not used unless the bone is being used
* with an IK method that supports them.
*
*/
USTRUCT(BlueprintType)
struct UE4IK_API FIKBone
{
	GENERATED_USTRUCT_BODY()
		
public:
	
	FIKBone()
		:
		BoneIndex(INDEX_NONE)
	{ }
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FBoneReference BoneRef;

	FIKBoneConstraint* GetConstraint();

	FCompactPoseBoneIndex BoneIndex;

public:

    // Check if this bone is valid, if not, attempt to initialize it. Return whether the bone is (after re-initialization if needed)
	bool InitIfInvalid(const FBoneContainer& RequiredBones);
	
	// Initialize this IK Bone. Must be called before use.
	bool Init(const FBoneContainer& RequiredBones);

	bool IsValid(const FBoneContainer& RequiredBones);

protected:

	UPROPERTY(EditAnywhere, Instanced, NoClear, Export, Category = "Settings")
	UIKBoneConstraintWrapper* Constraint;

};

/*
* Allows bones to be passed around in BP
*/
UCLASS(BlueprintType)
class UE4IK_API UIKBoneWrapper : public UObject
{
	
	GENERATED_BODY()

public: 

	UIKBoneWrapper(const FObjectInitializer& ObjectInitializer)
		: 
		Super(ObjectInitializer),
		bInitialized(false)
	{ }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FIKBone Bone;

public: 

	UFUNCTION(BlueprintCallable, Category = IK)
	void Initialize(FIKBone InBone);


	bool InitIfInvalid(const FBoneContainer& RequiredBones);
	
	bool Init(const FBoneContainer& RequiredBones);
	
	bool IsValid(const FBoneContainer& RequiredBones);
	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RangeLimitedIK")
	TArray<FIKBone> BonesRootToEffector;

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
UCLASS(BlueprintType)
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
	virtual bool InitIfInvalid(const FBoneContainer& RequiredBones);

	// Initialize all bones used in this chain. Must be called before use.
	virtual bool InitBoneReferences(const FBoneContainer& RequiredBones);

	// Check whether this chain is valid to use. Should be called in the IsValid method of your animnode.
	virtual bool IsValid(const FBoneContainer& RequiredBones);

protected:
	bool bInitialized;
};


UCLASS(BlueprintType)
class UE4IK_API URangeLimitedIKChainWrapper : public UIKChainWrapper
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FRangeLimitedIKChain Chain;

	UFUNCTION(BlueprintCallable, Category = IK)
	void Initialize(FRangeLimitedIKChain InChain);

	virtual bool InitIfInvalid(const FBoneContainer& RequiredBones);

	// Initialize all bones used in this chain. Must be called before use.
	virtual bool InitBoneReferences(const FBoneContainer& RequiredBones);

	// Check whether this chain is valid to use. Should be called in the IsValid method of your animnode.
	virtual bool IsValid(const FBoneContainer& RequiredBones);
};

