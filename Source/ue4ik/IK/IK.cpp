// Copyright (c) Henry Cooney 2017

#include "IK.h"

bool FIKModChain::InitIfInvalid(const FBoneContainer& RequiredBones)
{
	if (IsValid(RequiredBones))
	{
		return true;
	}

	InitAndAssignBones(RequiredBones);
	bool bValid = IsValid(RequiredBones);
	return bValid;
}

bool FIKModChain::InitBoneReferences(const FBoneContainer& RequiredBones)
{
	bool bInitSuccess = InitAndAssignBones(RequiredBones);
	bInitSuccess = bInitSuccess && RootBone.Init(RequiredBones) && EffectorBone.Init(RequiredBones);
	return bInitSuccess;
}

bool FIKModChain::IsValid(const FBoneContainer& RequiredBones)
{
	bool bValid = RootBone.IsValid(RequiredBones) && EffectorBone.IsValid(RequiredBones);
	bValid &= IsValidInternal(RequiredBones);
	return bValid;
}

bool FIKModChain::InitAndAssignBones(const FBoneContainer& RequiredBones)
{
	check(false);
	return false;
}

// Subclasses must implement this function so that all additional bones are tested for validity
bool FIKModChain::IsValidInternal(const FBoneContainer& RequiredBones)
{
	check(false);
	return false;
}
