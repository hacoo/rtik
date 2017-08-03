// Copyright(c) Henry Cooney 2017

#include "AnimNode_HumanoidArmTorsoAdjust.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "Utility/AnimUtil.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif

DECLARE_CYCLE_STAT(TEXT("IK Humanoid Arm Torso Adjust"), STAT_HumanoidArmTorsoAdjust_Eval, STATGROUP_Anim);

void FAnimNode_HumanoidArmTorsoAdjust::Initialize(const FAnimationInitializeContext & Context)
{
	Super::Initialize(Context);
	BaseComponentPose.Initialize(Context);
}

void FAnimNode_HumanoidArmTorsoAdjust::CacheBones(const FAnimationCacheBonesContext & Context)
{
	Super::CacheBones(Context);
	BaseComponentPose.CacheBones(Context);
}

void FAnimNode_HumanoidArmTorsoAdjust::UpdateInternal(const FAnimationUpdateContext & Context)
{
	BaseComponentPose.Update(Context);
	DeltaTime = Context.GetDeltaTime();	
}

void FAnimNode_HumanoidArmTorsoAdjust::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	SCOPE_CYCLE_COUNTER(STAT_HumanoidArmTorsoAdjust_Eval);

#if ENABLE_ANIM_DEBUG
	check(Output.AnimInstanceProxy->GetSkelMeshComponent());
#endif
	check(OutBoneTransforms.Num() == 0);

	// Input pin pointers are checked in IsValid -- don't need to check here

	USkeletalMeshComponent* SkelComp   = Output.AnimInstanceProxy->GetSkelMeshComponent();
	const USkeletalMeshSocket* TorsoPivotSocket = SkelComp->GetSocketByName(TorsoPivotSocketName);

	if (TorsoPivotSocket == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Could not evaluate humanoid arm torso adjustment -- torso pivot socket named %s could not be found"),
			*TorsoPivotSocketName.ToString());
#endif
		return;
	}

	if (Arm->Chain.Num() < 1)
	{
		return;
	}

	// Create two artificial 'bones'. The spine bone goes from the torso pivot to the neck,
	// the shoulder bone goes from the neck to  the shoulder ball joint.
	FTransform ShoulderCS = Output.Pose.GetComponentSpaceTransform(Arm->Chain[0].BoneIndex);

	// 'Local transform'? Not 'component transform'? Come on guys
	FTransform PivotCS = TorsoPivotSocket->GetSocketLocalTransform();

	// Neck is directly above pivot, at shoulder height
	FTransform NeckCS(PivotCS);
	NeckCS.AddToTranslation(FVector(0.0f, 0.0f, ShoulderCS.GetLocation().Z));


	// Set up augmented chain -- pivot and neck preceed the arm chain
	int32 NumBones = Arm->Chain.Num() + 2;
	TArray<FTransform> CSTransforms;
	CSTransforms.Reserve(NumBones);
	CSTransforms.Add(PivotCS);
	CSTransforms.Add(NeckCS);

	for (FIKBone& Bone : Arm->Chain.BonesRootToEffector)
	{
		CSTransforms.Add(Output.Pose.GetComponentSpaceTransform(Bone.BoneIndex));
	}

	// Set up pivot constraint. It can bend forward and backward. 
	





	


	




	

#if WITH_EDITOR
	if (bEnableDebugDraw)
	{
		UWorld* World = SkelComp->GetWorld();		
		FMatrix ToWorld = SkelComp->ComponentToWorld.ToMatrixNoScale();
	}
#endif // WITH_EDITOR
}

bool FAnimNode_HumanoidArmTorsoAdjust::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	
	if (Arm == nullptr)
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Humaonid Arm Torso Adjust was not valid to evaluate - an input wrapper was null"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;		
	}

	if (!Arm->IsValid(RequiredBones))
	{
#if ENABLE_IK_DEBUG_VERBOSE
		UE_LOG(LogIK, Warning, TEXT("Humaonid Arm Torso Adjust was not valid to evaluate - arm chain was not valid to evaluate"));		
#endif ENABLE_IK_DEBUG_VERBOSE
		return false;				
	}

	return true;
}

void FAnimNode_HumanoidArmTorsoAdjust::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	if (Arm == nullptr)
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Coud not initialize humanoid arm torso adjust - An input wrapper object was null"));
#endif // ENABLE_IK_DEBUG
		return;
	}
	
	if (!Arm->InitBoneReferences(RequiredBones))
	{
#if ENABLE_IK_DEBUG
		UE_LOG(LogIK, Warning, TEXT("Could not initialize arm chain in humanoid arm torso adjust"));
#endif // ENABLE_IK_DEBUG
		return;
	}
}
