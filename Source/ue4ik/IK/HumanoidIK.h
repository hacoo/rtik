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
struct FFabrikHumanoidLegChain : public FFabrikIKChain
{
	GENERATED_USTRUCT_BODY()

public:

	FFabrikHumanoidLegChain()
		:
		FootRadius(10.0f),
		ToeRadius(5.0f),
		TotalChainLength(0.0f),
		bInitOk(false)
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

	float GetTotalChainLength() const;
	

protected:
	bool bInitOk;
	virtual bool IsValidInternal(const FBoneContainer& RequiredBones) override;
	virtual bool InitAndAssignBones(const FBoneContainer& RequiredBones) override;

	// Total length of all bones in the chain (thigh, shin, and foot bones).
    // Does not include foot or toe radius.
	float TotalChainLength;
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


USTRUCT()
struct FHumanoidIK 
{
	GENERATED_USTRUCT_BODY()

	/*
    * Does traces from foot, toe, etc. Traces are somewhat expensive, so try to
    * call this once per graph and reuse the result between IK nodes.
    */
	static void HumanoidIKLegTrace(ACharacter* Character, 
		FCSPose<FCompactPose>& MeshBases,
		FFabrikHumanoidLegChain& LegChain,
		FHumanoidIKTraceData& OutTraceData);
};

