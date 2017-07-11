// Copyright (c) Henry Cooney 2017

#include "HumanoidIK.h"
#include "Utility/AnimUtil.h"
#include "Utility/TraceUtil.h"

void FHumanoidIK::HumanoidIKLegTrace(ACharacter* Character,
	FCSPose<FCompactPose>& MeshBases,
	FFabrikHumanoidLegChain& LegChain,
	FHumanoidIKTraceData& OutTraceData)
{
	// Traces to find floor points below foot bone and toe. Traces from character capsule midpoint
    // to the max step-down height.

    // Trace direction is downward axis of the character.

	if (Character == nullptr)
	{
		UE_LOG(LogIK, Warning, TEXT("FHumanoidIK::HumanoidIKLegTrace -- Leg trace failed, Character was invalid"));
		return;
	}

	UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent();
	USkeletalMeshComponent* SkelComp = Character->GetMesh();
	UWorld* World = Character->GetWorld();
	const FBoneContainer& RequiredBones = MeshBases.GetPose().GetBoneContainer();	
	FVector TraceDirection = -1 * Character->GetActorUpVector();

	// Do foot trace:

	FVector FootLocation = FAnimUtil::GetBoneWorldLocation(*SkelComp,
		MeshBases, 
		LegChain.ShinBone.BoneIndex);

	FVector ToeLocation = FAnimUtil::GetBoneWorldLocation(*SkelComp,
		MeshBases, 
		LegChain.FootBone.BoneIndex);
	
	//UTraceUtil::LineTrace(World, &Character, )
	



	//FVector FootTraceStart =

}



bool FFabrikHumanoidLegChain::IsValidInternal(const FBoneContainer& RequiredBones)
{
	bool bValid = HipBone.IsValid(RequiredBones)
		&& ThighBone.IsValid(RequiredBones)
		&& ShinBone.IsValid(RequiredBones)
		&& FootBone.IsValid(RequiredBones);

	return bValid;
}


float FFabrikHumanoidLegChain::GetTotalChainLength() const
{
	return TotalChainLength;
}


bool FFabrikHumanoidLegChain::InitAndAssignBones(const FBoneContainer& RequiredBones)
{
	TotalChainLength = 0.0f;
	bInitOk = true;
	
	EffectorBone = ShinBone;
	RootBone = HipBone;
	
	if (!HipBone.Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialized IK leg chain - Hip Bone invalid"));
#endif // ENABLE_IK_DEBUG			
		bInitOk = false;
	}
	
	if (!ThighBone.Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialized IK leg chain - Thigh Bone invalid"));
#endif // ENABLE_IK_DEBUG
		bInitOk = false;
	}
	
	if (!ShinBone.Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialized IK leg chain - Shin Bone invalid"));
#endif // ENABLE_IK_DEBUG			
		bInitOk = false;
	}
	
	if (!FootBone.Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialized IK leg chain - Foot Bone invalid"));
#endif // ENABLE_IK_DEBUG
		bInitOk = false;
	}
	
	// Compute extended chain length
	if (bInitOk)
	{
		const TArray<FTransform>& RefTransforms = RequiredBones.GetRefPoseArray();
		FVector ThighVec = RefTransforms[HipBone.BoneRef.BoneIndex].GetLocation() -
				RefTransforms[ThighBone.BoneRef.BoneIndex].GetLocation();
		FVector ShinVec = RefTransforms[ThighBone.BoneRef.BoneIndex].GetLocation() -
			RefTransforms[ShinBone.BoneRef.BoneIndex].GetLocation();
		FVector FootVec = RefTransforms[ShinBone.BoneRef.BoneIndex].GetLocation() -
			RefTransforms[FootBone.BoneRef.BoneIndex].GetLocation();
		TotalChainLength = ThighVec.Size() + ShinVec.Size() + FootVec.Size();
	}
	
	return bInitOk;
}