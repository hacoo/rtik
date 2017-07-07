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

	bool InitBoneReferences(const FBoneContainer& RequiredBones)
	{
		bool bInitSuccess = InitAndAssignBones(RequiredBones);
		if (bInitSuccess)
		{
			FabrikSolver.EffectorRotationSource = BRS_KeepComponentSpaceRotation;
			FabrikSolver.EffectorTransformSpace = BCS_WorldSpace;
			FabrikSolver.MaxIterations = MaxIterations;
			FabrikSolver.Precision = Precision;
		}
		else
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
