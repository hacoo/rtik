// Copyright (c) Henry Cooney 2017

#include "HumanoidIK.h"
#include "Utility/AnimUtil.h"
#include "Utility/TraceUtil.h"
#include "Components/SkeletalMeshComponent.h"

void FHumanoidIK::HumanoidIKLegTrace(ACharacter* Character,
	FCSPose<FCompactPose>& MeshBases,
	FHumanoidLegChain& LegChain,
	FIKBone& PelvisBone,
	float MaxPelvisAdjustHeight,
	FHumanoidIKTraceData& OutTraceData,
	bool bEnableDebugDraw)
{
	// Traces to find floor points below foot bone and toe. 

    // Each trace proceeds downward in a vertical line through the foot / toe bone. The start point is the
    // pelvis height, foot height, or toe height, whichever is greatest. The end point is determined by the max leg exention length.

    // Trace direction is downward axis of skeletal mesh component.

	if (Character == nullptr)
	{		
		return;
	}

	USkeletalMeshComponent* SkelComp = Character->GetMesh();
	UWorld* World = Character->GetWorld();
	const FBoneContainer& RequiredBones = MeshBases.GetPose().GetBoneContainer();	
	FVector TraceDirection = -1 * Character->GetActorUpVector();

	// All calcuations done in CS; will be translated to world space for final trace
	FVector PelvisLocation = FAnimUtil::GetBoneCSLocation(*SkelComp,
		MeshBases,
		PelvisBone.BoneIndex);

	FVector FootLocation = FAnimUtil::GetBoneCSLocation(*SkelComp, MeshBases, LegChain.ShinBone.BoneIndex);
	FVector ToeLocation = FAnimUtil::GetBoneCSLocation(*SkelComp, MeshBases, LegChain.FootBone.BoneIndex);

	float TraceStartHeight = FMath::Max3(FootLocation.Z + LegChain.FootRadius,
		ToeLocation.Z + LegChain.ToeRadius,
		PelvisLocation.Z);
	float TraceEndHeight = PelvisLocation.Z - (LegChain.GetTotalChainLength() + LegChain.FootRadius + LegChain.ToeRadius + MaxPelvisAdjustHeight);

	FVector FootTraceStart(FootLocation.X, FootLocation.Y, TraceStartHeight);
	FVector FootTraceEnd(FootLocation.X, FootLocation.Y, TraceEndHeight);

	FVector ToeTraceStart(ToeLocation.X, ToeLocation.Y, TraceStartHeight);
	FVector ToeTraceEnd(ToeLocation.X, ToeLocation.Y, TraceEndHeight);

	// Convert to world space for tracing
	FTransform ComponentToWorld = SkelComp->ComponentToWorld;
	FootTraceStart = ComponentToWorld.TransformPosition(FootTraceStart);
	FootTraceEnd   = ComponentToWorld.TransformPosition(FootTraceEnd);
	ToeTraceStart  = ComponentToWorld.TransformPosition(ToeTraceStart);
	ToeTraceEnd    = ComponentToWorld.TransformPosition(ToeTraceEnd);
	
	UTraceUtil::LineTrace(World,
		Character,
		FootTraceStart,
		FootTraceEnd,
		OutTraceData.FootHitResult,
		ECC_Pawn,
		false,
		bEnableDebugDraw);

	UTraceUtil::LineTrace(World,
		Character,
		ToeTraceStart,
		ToeTraceEnd,
		OutTraceData.ToeHitResult,
		ECC_Pawn,
		false,
		bEnableDebugDraw);
}

bool FHumanoidLegChain::IsValid(const FBoneContainer& RequiredBones)
{
	bool bValid = HipBone.IsValid(RequiredBones)
		&& ThighBone.IsValid(RequiredBones)
		&& ShinBone.IsValid(RequiredBones)
		&& FootBone.IsValid(RequiredBones);

	return bValid;
}

float FHumanoidLegChain::GetTotalChainLength() const
{
	return TotalChainLength;
}

bool FHumanoidLegChain::FindWithinFootRotationLimit(const USkeletalMeshComponent& SkelComp,
	const FHumanoidIKTraceData& TraceData,
	float& OutAngleRad) const
{

	if (TraceData.FootHitResult.GetActor() == nullptr ||
		TraceData.ToeHitResult.GetActor() == nullptr)
	{
		return false;
	}

	FVector ToCS = -1 * SkelComp.GetComponentLocation();

	FVector FootFloorCS = ToCS + TraceData.FootHitResult.ImpactPoint;
	FVector ToeFloorCS  = ToCS + TraceData.ToeHitResult.ImpactPoint;
	
	FVector FloorSlopeVec = ToeFloorCS - FootFloorCS;
	FVector FloorFlatVec(FloorSlopeVec);
	FloorFlatVec.Z = 0.0f;
	
	if(!FloorSlopeVec.Normalize() || !FloorFlatVec.Normalize())
	{
		OutAngleRad = 0.0f;
		return false;
	}
   
	OutAngleRad = FMath::Acos(FVector::DotProduct(FloorFlatVec, FloorSlopeVec));
	float RequiredRotationDeg = FMath::RadiansToDegrees(OutAngleRad);

	if (RequiredRotationDeg > MaxFootRotationDegrees)
	{
		return false;
	}
	
	return true;
}

bool FHumanoidLegChain::GetIKFloorPointCS(const USkeletalMeshComponent& SkelComp,
	const FHumanoidIKTraceData& TraceData,
	FVector& OutTraceLocationCS) const 
{
	FVector ToCS        = -1 * SkelComp.GetComponentLocation();
	FVector FootFloorCS = ToCS + TraceData.FootHitResult.ImpactPoint;
	FVector ToeFloorCS  = ToCS + TraceData.ToeHitResult.ImpactPoint;

	// If one of the trace results is invalid, don't rotate, and use the other one
	if (TraceData.FootHitResult.GetActor() == nullptr || TraceData.ToeHitResult.GetActor() == nullptr)
	{
		if (TraceData.FootHitResult.GetActor() != nullptr)
		{
			OutTraceLocationCS = FootFloorCS;
		}
		else if (TraceData.ToeHitResult.GetActor() != nullptr)
		{
			OutTraceLocationCS = ToeFloorCS;
		}

#if ENABLE_IK_DEBUG_VERBOSE
		else
		{
			UE_LOG(LogRTIK, Warning, TEXT("Warning, GetIKFloorPointCS was called on an invalid trace result. The output floor point may be invalid."));
			// check (false);
		}
#endif // ENABLE_IK_DEBUG
		
		return false;
	}

	float Unused;
	// If within foot rotation limit, always use the foot. Otherwise, use the higher point and the foot shouldn't rotate.
	bool bWithinRotationLimit = FindWithinFootRotationLimit(SkelComp, TraceData, Unused);
	
	if (bWithinRotationLimit)
	{
		OutTraceLocationCS = FootFloorCS;
	}
	else
	{
		OutTraceLocationCS = FootFloorCS.Z > ToeFloorCS.Z ? FootFloorCS : ToeFloorCS;
	}

	return bWithinRotationLimit;
}

bool FHumanoidLegChain::InitBoneReferences(const FBoneContainer& RequiredBones)
{
	TotalChainLength = 0.0f;
	bInitOk = true;
		
	if (!HipBone.Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialized IK leg chain - Hip Bone invalid"));
#endif // ENABLE_IK_DEBUG			
		bInitOk = false;
	}
	
	if (!ThighBone.Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialized IK leg chain - Thigh Bone invalid"));
#endif // ENABLE_IK_DEBUG
		bInitOk = false;
	}
	
	if (!ShinBone.Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialized IK leg chain - Shin Bone invalid"));
#endif // ENABLE_IK_DEBUG			
		bInitOk = false;
	}
	
	if (!FootBone.Init(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogRTIK, Warning, TEXT("Could not initialized IK leg chain - Foot Bone invalid"));
#endif // ENABLE_IK_DEBUG
		bInitOk = false;
	}
	
	// Compute extended chain length
	if (bInitOk)
	{
		const TArray<FTransform>& RefTransforms = RequiredBones.GetRefPoseArray();

		FVector HipLoc   = RefTransforms[HipBone.BoneRef.BoneIndex].GetLocation();
		FVector KneeLoc  = RefTransforms[ThighBone.BoneRef.BoneIndex].GetLocation();
		FVector AnkleLoc = RefTransforms[ShinBone.BoneRef.BoneIndex].GetLocation();
		FVector ToeLoc   = RefTransforms[FootBone.BoneRef.BoneIndex].GetLocation();
		
		FVector ThighVec = KneeLoc - HipLoc;
		FVector ShinVec  = AnkleLoc - KneeLoc;
		FVector FootVec  = ToeLoc - AnkleLoc;

		float ThighSize  = ThighVec.Size();
		float ShinSize   = ShinVec.Size();
		float FootSize   = FootVec.Size();

		TotalChainLength = ThighSize + ShinSize + FootSize;
	}
	
	return bInitOk;
}

/*
bool FHumanoidArmChain::InitBoneReferences(const FBoneContainer & RequiredBones)
{
	// Automatically set up chain
	BonesRootToEffector.Empty();
	BonesRootToEffector.Reserve(2);
	BonesRootToEffector.Add(UpperArmBone);
	BonesRootToEffector.Add(LowerArmBone);

	UpperArmBone.Init(RequiredBones);
	LowerArmBone.Init(RequiredBones);
	
	Super::InitBoneReferences(RequiredBones);
}

bool FHumanoidArmChain::IsValid(const FBoneContainer & RequiredBones)
{
	bool bValid = true;
	bValid &= UpperArmBone.IsValid(RequiredBones);
	bValid &= LowerArmBone.IsValid(RequiredBones);
	bValid &= Super::InitBoneReferences(RequiredBones);

	return bValid;
}
*/
