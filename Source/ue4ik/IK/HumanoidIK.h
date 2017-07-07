// Copyright (c) Henry Cooney 2017

#pragma once

#include "IK.h"
#include "HumanoidIK.generated.h"

/*
 * Basic structs, etc for humanoid biped IK. 
*/

/*
* Represents a humanoid leg, with a hip bone, thigh bone, and shin bone. 
*/
USTRUCT(BlueprintType)
struct FFabrikHumanoidLegChain : public FFabrikIKChain
{
	GENERATED_USTRUCT_BODY()

public:

	FFabrikHumanoidLegChain()
		:
		FootRadius(0.0f)
	{ }

	// Distance between the bottom of the shin bone and the bottom surface of the foot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float FootRadius;
	
	// Connects from pelvis to upper leg bone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone HipBone;

	// Connects from end of hip to top of knee
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone ThighBone;

	// Connects from bottom of knee to top of foot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone ShinBone;

	virtual bool InitAndAssignBones(const FBoneContainer& RequiredBones)
	{
		bool bInitOk = true;
		if (!HipBone.Init(RequiredBones))
		{
			UE_LOG(LogIK, Warning, TEXT("Could not initialized IK leg chain - Hip Bone invalid"));
			bInitOk = false;
		}

		if (!ThighBone.Init(RequiredBones))
		{
			UE_LOG(LogIK, Warning, TEXT("Could not initialized IK leg chain - Thigh Bone invalid"));
			bInitOk = false;
		}

		if (!ShinBone.Init(RequiredBones))
		{
			UE_LOG(LogIK, Warning, TEXT("Could not initialized IK leg chain - Shin Bone invalid"));
			bInitOk = false;
		}

		EffectorBone = ShinBone;
		RootBone = HipBone;
   
		return bInitOk;		
	}
};

/*
* Holds trace data used in IK. Don't make one yourself, use the Humanoid IK Trace node to create one.
*/
USTRUCT(BlueprintType)
struct FHumanoidIKTraceData 
{
	GENERATED_USTRUCT_BODY()

public:

	FHitResult FootHitResult;
	FHitResult LegHitResult;
};


UCLASS()
class UHumanoidIKLibrary : public UBlueprintFunctionLibrary
{

	GENERATED_BODY()

	/*
    * Does traces from foot, toe, etc. Traces are somewhat expensive, so try to
    * call this once per graph and reuse the result between IK nodes.
    */
	UFUNCTION(BlueprintPure, Category = IK)
	void HumanoidIKLegTrace(const FFabrikHumanoidLegChain& LegChain, FHumanoidIKTraceData& OutTraceData);
};

