// Copyright (c) Henry Cooney 2017

#include "IK.h"

bool FFabrikIKChain::InitIfInvalid(const FBoneContainer& RequiredBones)
{
	if (IsValid(RequiredBones))
	{
		return true;
	}

	InitAndAssignBones(RequiredBones);
	bool bValid = IsValid(RequiredBones);
	return bValid;
}

bool FFabrikIKChain::InitBoneReferences(const FBoneContainer& RequiredBones)
{
	bool bInitSuccess = InitAndAssignBones(RequiredBones);
	bInitSuccess = bInitSuccess && RootBone.Init(RequiredBones) && EffectorBone.Init(RequiredBones);
	if (bInitSuccess)
	{
		FabrikSolver.EffectorRotationSource = BRS_KeepComponentSpaceRotation;
		FabrikSolver.EffectorTransformSpace = BCS_WorldSpace;
		FabrikSolver.MaxIterations = MaxIterations;
		FabrikSolver.Precision = Precision;
	}
	return bInitSuccess;
}

bool FFabrikIKChain::IsValid(const FBoneContainer& RequiredBones)
{
	bool bValid = RootBone.IsValid(RequiredBones) && EffectorBone.IsValid(RequiredBones);
	bValid &= IsValidInternal(RequiredBones);
	return bValid;
}

bool FFabrikIKChain::InitAndAssignBones(const FBoneContainer& RequiredBones)
{
	check(false);
	return false;
}

// Subclasses must implement this function so that all additional bones are tested for validity
bool FFabrikIKChain::IsValidInternal(const FBoneContainer& RequiredBones)
{
	check(false);
	return false;
}
