// Copyright (c) Henry Cooney 2017

#include "IK.h"

bool FIKModChain::InitIfInvalid(const FBoneContainer& RequiredBones)
{
	if (IsValid(RequiredBones))
	{
		return true;
	}

	InitBoneReferences(RequiredBones);
	bool bValid = IsValid(RequiredBones);
	return bValid;
}

bool FIKModChain::InitBoneReferences(const FBoneContainer& RequiredBones)
{
	return false;
}

bool FIKModChain::IsValid(const FBoneContainer& RequiredBones)
{
	return false;
}

uint8 IKBoneAxisToAxis(EIKBoneAxis InBoneAxis)
{
	if (InBoneAxis == EIKBoneAxis::IKBA_X)
	{
		return EAxis::X;
	}
	else if (InBoneAxis == EIKBoneAxis::IKBA_Y)
	{
		return EAxis::Y;
	}
	else if (InBoneAxis == EIKBoneAxis::IKBA_Z)
	{
		return EAxis::Z;
	}
	{
		return EAxis::None;
	}
}

bool FRangeLimitedIKChain::InitBoneReferences(const FBoneContainer & RequiredBones)
{
	bool bValid = true;
	for (int32 i = 1; i < BonesEffectorToRoot.Num(); ++i)
	{
		FIKBone& Bone = BonesEffectorToRoot[i];
		if (!Bone.Init(RequiredBones))
		{
			bValid = false;
		}
		FIKBone& PreviousBone = BonesEffectorToRoot[i-1];
		if (RequiredBones.GetParentBoneIndex(PreviousBone.BoneIndex) != Bone.BoneIndex)
		{
#if ENABLE_IK_DEBUG
			UE_LOG(LogIK, Warning, TEXT("Could not initialized range limited IK chain - bone named %s was not preceeded by its skeletal parent"),
				*(Bone.BoneRef.BoneName.ToString()));
#endif // ENABLE_IK_DEBUG
			bValid = false;
		}
	}

	return bValid;
}

bool FRangeLimitedIKChain::IsValid(const FBoneContainer & RequiredBones)
{
	bool bValid = true;
	for (FIKBone& Bone : BonesEffectorToRoot)
	{
		bValid &= Bone.IsValid(RequiredBones);
	}
	return bValid;
}
