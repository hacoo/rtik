// Copyright (c) Henry Cooney 2017

// Provides animation utility functions (many blueprintable). All functions defined here are pure.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Public/BonePose.h"
#include "AnimUtil.generated.h"

/**
 * Animation utility functions
 */
USTRUCT()
struct UE4IK_API FAnimUtil 
{
	GENERATED_BODY()
	
public:

	// Get worldspace location of a bone
	static FVector GetBoneWorldLocation(USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& MeshBases, FCompactPoseBoneIndex BoneIndex);

	// Get worldspace transform of a bone
	static FTransform GetBoneWorldTransform(USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& MeshBases, FCompactPoseBoneIndex BoneIndex);

	// Get component space location of a bone
	static FVector GetBoneCSLocation(USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& MeshBases, FCompactPoseBoneIndex BoneIndex);

	// Get component space transform of a bone
	static FTransform GetBoneCSTransform(USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& MeshBases, FCompactPoseBoneIndex BoneIndex);

};
