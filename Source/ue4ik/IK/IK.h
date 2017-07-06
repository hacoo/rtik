// Copyright (c) Henry Cooney 2017

/*
* Contains basic IK structures.
*/

#pragma once

#include "ue4ik.h"
#include "AnimNode_SkeletalControlBase.h"
#include "IK.generated.h"


/*
* A bone used in IK
*/
USTRUCT()
struct FIKBone
{
	GENERATED_USTRUCT_BODY()

public:
	
	FIKBone()
		:
		bInitSuccess(false)
	{ }

	FIKBone& operator= (const FIKBone& Other)
	{		
		BoneRef = Other.BoneRef;
		bInitSuccess = Other.bInitSuccess;
		return *this;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FBoneReference BoneRef;
	
	bool Init(const FBoneContainer& RequiredBones)
	{
		if (BoneRef.Initialize(RequiredBones))
		{
			return true;
		}
		else
		{
			UE_LOG(LogIK, Warning, TEXT("FIKBone::Init -- Bone initialization failed"));
			return false;
		}
	}

	bool IsInitialized()
	{
		return bInitSuccess;
	}

private:
	bool bInitSuccess;

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
	bInitSuccess(false)
	{ }

	FIKBone RootBone;
	FIKBone EffectorBone;	
	

	bool InitBoneReferences(const FBoneContainer& RequiredBones)
	{
		bool bInitSuccess = InitAndAssignBones(RequiredBones);
		if (!bInitSuccess)
		{
			UE_LOG(LogIK, Warning, TEXT("UFabrikIKChain::InitBoneReferences -- Chain contains invalid bone references and could not be initialized"));
		}
		return bInitSuccess;
	}

private:
	// Subclasses must implement this function so that:
    // - All additional bones are initialized
    // - RootBone and EffectorBone are assigned as needed
	virtual bool InitAndAssignBones(const FBoneContainer& RequiredBones)
	{
		return true;
	}
	bool bInitSuccess;

};


/*
* Represents a humanoid leg, with a hip bone, thigh bone, and shin bone. 
*/
USTRUCT(BlueprintType)
struct FFabrikBipedLegChain : public FFabrikIKChain
{
	GENERATED_USTRUCT_BODY()

public:

	FFabrikBipedLegChain()
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