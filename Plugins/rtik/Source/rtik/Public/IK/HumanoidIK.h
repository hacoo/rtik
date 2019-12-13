// Copyright (c) Henry Cooney 2017

#pragma once

#include "IK.h"
#include "BonePose.h"
#include "HumanoidIK.generated.h"


/*
* Basic structs, etc for humanoid biped IK.
*/

/*
* Represents a humanoid leg, with a hip bone, thigh bone, and shin bone.
*/
USTRUCT(BlueprintType)
struct RTIK_API FHumanoidLegChain : public FIKModChain
{
	GENERATED_USTRUCT_BODY()
		
public:
	
	FHumanoidLegChain()
		:
		FootRadius(10.0f),
		ToeRadius(5.0f),
		MaxFootRotationDegrees(30.0f),
		bInitOk(false),
		TotalChainLength(0.0f)
	{ }
	
	// Distance between the bottom of the shin bone and the bottom surface of the foot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float FootRadius;
	
	// Distance between
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float ToeRadius;
	
	// Connects from pelvis to upper leg bone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone HipBone;
	
	// Connects from end of hip to top of knee
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone ThighBone;
	
	// Connects from bottom of knee to top of foot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone ShinBone;
	
	// Connects from bottom of shin to start of the toe
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone FootBone;

	// Maximum rotation, in degrees, that the foot will make in order to match the floor angle.
	// If a greater rotation than this is required, the foot will remain flat and prefer
	// to IK onto the higher ground point.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MaxFootRotationDegrees;

	// Gets fully-extended length of entire chain (including foot bone)
	float GetTotalChainLength() const;

	// Determines whether the slope of the floor (sampled at foot / toe trace points) is 
	// within MaxFootRotationDegrees.
	// @param TraceData - Trace data for this leg. Must have been updated this tick.
	// @param OutAngleRad - Returns the UNSIGNED angle, in radians, between the slope of the floor and flat ground. 
	// @return - true if floor slope is within rotation limit and the foot should rotate, else false.	
	bool FindWithinFootRotationLimit(const USkeletalMeshComponent& SkelComp,
		const FHumanoidIKTraceData& TraceData,
		float& OutAngleRad) const;
	
	// Gets the relevant trace floor point for IK, converts it to component space, and returns in OutFloorLocationCS.
	// @param TraceData - Trace data for this leg. Must have been updated this tick.
	// @param OutTraceLocationCS - The trace point (either below the foot or the toe) to use.
	// @return - True if the low IK target was returned, and the foot should rotate to match floor slope. False
	// if the high IK target was returned, and the foot shouldn't rotate.
	bool GetIKFloorPointCS(const USkeletalMeshComponent& SkelComp,
		const FHumanoidIKTraceData& TraceData, FVector& OutFloorLocationCS) const;

	// FIKModChain interface
	virtual bool InitBoneReferences(const FBoneContainer& RequiredBones) override;
	virtual bool IsValid(const FBoneContainer& RequiredBones) override;
	// end FIKModChain interface
   	
protected:
	bool bInitOk;	
	// Total length of all bones in the chain (thigh, shin, and foot bones).
    // Does not include foot or toe radius.
	float TotalChainLength;
};

/*
// Represents a humanoid arm, with UpperArm (shoulder to elbow) and LowerArm (elbow to wrist) bones
// Basically this is a convenience wrapper, hiding details of FRangeLimitedIKChain
USTRUCT(BlueprintType, hidecategories = ("RangeLimitedIK"))
struct RTIK_API FHumanoidArmChain : public FRangeLimitedIKChain
{
	GENERATED_USTRUCT_BODY()

	// Connects from shoulder to elbow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HumanoidArmChain")
	FIKBone UpperArmBone;
	
	// Connects from elbow to wrist
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HumanoidArmChain")
	FIKBone LowerArmBone;
	
	// Range of motion of the shoulder joint. Shoulder functions as a Conic constraint
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HumanoidArmChain", meta = (UIMin = 0.0f, UIMax = 180.0f))
	// float ShouldROMDegrees;

	// Begin FRangeLimitedIKChain interface
	virtual bool InitBoneReferences(const FBoneContainer& RequiredBones) override;
	virtual bool IsValid(const FBoneContainer& RequiredBones) override;
	// End FRangeLimitedIKChain interface

};
*/


/*
* Holds trace data used in leg IK
*/
USTRUCT(BlueprintType)
struct RTIK_API FHumanoidIKTraceData
{
	GENERATED_USTRUCT_BODY()
		
public:
	
	FHitResult FootHitResult;
	FHitResult ToeHitResult;
};

/*
* Wrapper for passing trace data around in BP. The trace node may write into the struct contained within!
*/
UCLASS(BlueprintType, EditInlineNew)
class RTIK_API UHumanoidIKTraceData_Wrapper : public UObject
{
	GENERATED_BODY()

public: 
	
	UHumanoidIKTraceData_Wrapper(const FObjectInitializer& ObjectInitializer)
		:
		Super(ObjectInitializer),
		bUpdatedThisTick(false)
	{ }

	// Data in this class should be updated each frame before use. This is handled
	// by the IKLegTrace class, which will ensure that this wrapper is marked as stale
	// until it is updated.

	// Gets trace data stored in this wrapper. Trace data should be updated by
	// before this is called by placing a trace node earlier in the animgraph. 
	// If an update function is not called, and the data is stale, the stale data is returned
	// and a warning is printed to console.	
	UFUNCTION(BlueprintCallable, Category = IK)
	FHumanoidIKTraceData& GetTraceData()
	{
#if ENABLE_IK_DEBUG
		if (!bUpdatedThisTick)
		{
			UE_LOG(LogRTIK, Warning, TEXT("Warning -- Trace data was used before it was updated and may be stale. Use a trace node (e.g., IK Humanoid Leg Trace) to update your trace data early in the animgraph, before it is used!"));
		}
#endif // ENABLE_IK_DEBUG
		return TraceData;
	}

	// Trace classes using this wrapper are declared as friends so they can directly update data and set bUpdatedThisTick
	friend struct FAnimNode_IKHumanoidLegTrace;

protected:
	bool bUpdatedThisTick;
	FHumanoidIKTraceData TraceData;
};

/*
* Wrapper class for passing around in BP
*/
UCLASS(BlueprintType, EditInlineNew)
class RTIK_API UHumanoidLegChain_Wrapper : public UIKChainWrapper
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FHumanoidLegChain Chain;

	UFUNCTION(BlueprintCallable, Category = IK)
	void Initialize(FHumanoidLegChain InChain) 
	{		
		Chain = InChain;
		bInitialized = true;
	}

	virtual bool InitIfInvalid(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
#if ENABLE_IK_DEBUG
			UE_LOG(LogRTIK, Warning, TEXT("Humanoid IK Leg Chain wrapper was not initialized -- make sure you call Initialize function in blueprint before use"));
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
			UE_LOG(LogRTIK, Warning, TEXT("Humanoid IK Leg Chain wrapper was not initialized -- make sure you call Initialize function in blueprint before use"));
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
/*
* Wrapper class for passing around in BP
UCLASS(BlueprintType, EditInlineNew)
class RTIK_API UHumanoidArmChain_Wrapper : public UIKChainWrapper
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FHumanoidArmChain Chain;

	UFUNCTION(BlueprintCallable, Category = IK)
	void Initialize(FHumanoidArmChain InChain) 
	{		
		Chain = InChain;
		bInitialized = true;
	}

	virtual bool InitIfInvalid(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
#if ENABLE_IK_DEBUG
			UE_LOG(LogRTIK, Warning, TEXT("Humanoid IK Arm Chain wrapper was not initialized -- make sure you call Initialize function in blueprint before use"));
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
			UE_LOG(LogRTIK, Warning, TEXT("Humanoid IK Arm Chain wrapper was not initialized -- make sure you call Initialize function in blueprint before use"));
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
*/


/*
* Contains Humanoid IK utility functions
*/
USTRUCT()
struct FHumanoidIK
{
	GENERATED_USTRUCT_BODY()
		
/*
* Does traces from foot and toe to the floor
*/
static void HumanoidIKLegTrace(ACharacter* Character,
	FCSPose<FCompactPose>& MeshBases,
	FHumanoidLegChain& LegChain,
	FIKBone& PelvisBone,
	float MaxPelvisAdjustHeight,
	FHumanoidIKTraceData& OutTraceData,
	bool bEnableDebugDraw = false);
};