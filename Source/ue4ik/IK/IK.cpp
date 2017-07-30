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