// Copyright (c) Henry Cooney 2017

// Provides animation utility functions (many blueprintable). All functions defined here are pure.

#include "rtik.h"
#include "AnimUtil.h"
#include "AnimationRuntime.h"


// Get the world space location vector for a bone
FVector FAnimUtil::GetBoneWorldLocation(USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& MeshBases, FCompactPoseBoneIndex BoneIndex)
{
	FTransform BoneTransform = MeshBases.GetComponentSpaceTransform(BoneIndex);
	FAnimationRuntime::ConvertCSTransformToBoneSpace(SkelComp.GetComponentTransform(), MeshBases, BoneTransform, BoneIndex, BCS_WorldSpace);
	return BoneTransform.GetLocation();
}

// Get the world space transform for a bone
FTransform FAnimUtil::GetBoneWorldTransform(USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& MeshBases, FCompactPoseBoneIndex BoneIndex)
{
	FTransform BoneTransform = MeshBases.GetComponentSpaceTransform(BoneIndex);
	FAnimationRuntime::ConvertCSTransformToBoneSpace(SkelComp.GetComponentTransform(), MeshBases, BoneTransform, BoneIndex, BCS_WorldSpace);
	return BoneTransform;
}

// Get the world space location vector for a bone
FVector FAnimUtil::GetBoneCSLocation(USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& MeshBases, FCompactPoseBoneIndex BoneIndex)
{
	return MeshBases.GetComponentSpaceTransform(BoneIndex).GetLocation();
}

// Get the world space transform for a bone
FTransform FAnimUtil::GetBoneCSTransform(USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& MeshBases, FCompactPoseBoneIndex BoneIndex)
{
	return MeshBases.GetComponentSpaceTransform(BoneIndex);
}
