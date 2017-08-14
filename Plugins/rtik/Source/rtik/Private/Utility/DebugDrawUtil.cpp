// Copyright (c) Henry Cooney 2017

#include "rtik.h"
#include "DebugDrawUtil.h"
#include "AnimUtil.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Async.h"

void FDebugDrawUtil::DrawLine(UWorld* World, const FVector& Start, const FVector& Finish, const FLinearColor& Color, float Duration, float Thickness)
{
	AsyncTask(ENamedThreads::GameThread, [World, Start, Finish, Color, Duration, Thickness]() {
		UKismetSystemLibrary::DrawDebugLine(Cast<UObject>(World), Start, Finish, Color, Duration, Thickness);
	});
}

void FDebugDrawUtil::DrawSphere(UWorld* World, const FVector& Center, const FLinearColor& Color, float Radius, int32 Segments, float Duration, float Thickness)
{ 
	AsyncTask(ENamedThreads::GameThread, [World, Center, Radius, Segments, Color, Duration, Thickness]() {
		UKismetSystemLibrary::DrawDebugSphere(Cast<UObject>(World), Center, Radius, Segments, Color, Duration, Thickness);
	});
}

void FDebugDrawUtil::DrawString(UWorld* World, const FVector& Location, const FString& Text, AActor * BaseActor, const FColor& Color, float Duration)
{
	AsyncTask(ENamedThreads::GameThread, [World, Location, Text, BaseActor, Color, Duration]() {
		UKismetSystemLibrary::DrawDebugString(Cast<UObject>(World), Location, Text, BaseActor, Color, Duration);
	});
}

void FDebugDrawUtil::DrawPlane(UWorld* World, const FVector& PlaneBase, const FVector& PlaneNormal,
	float Size, const FLinearColor& Color, bool bDrawNormal, float Duration)
{
	if (bDrawNormal)
	{
		DrawVector(World, PlaneBase, PlaneNormal, Color);
	}

	FPlane Plane(PlaneBase, PlaneNormal);
	AsyncTask(ENamedThreads::GameThread, [World, Plane, PlaneBase, Size, Color, Duration]() {		
		UKismetSystemLibrary::DrawDebugPlane(Cast<UObject>(World), Plane, PlaneBase, Size, Color, Duration);
	});
}

void FDebugDrawUtil::DrawVector(UWorld * World, const FVector& Base, FVector Direction, const FLinearColor& Color, float Length, float Duration, float Thickness)
{
	if (!Direction.Normalize())
	{
		return;
	}
	Direction *= Length;
	DrawLine(World, Base, Base + Direction, Color, Duration, Thickness);
}

void FDebugDrawUtil::DrawBone(UWorld* World, USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& Pose,
	const FCompactPoseBoneIndex& ChildBone, const FLinearColor& Color, float Duration, float Thickness)
{

	int32 ChildIndex = ChildBone.GetInt();
	FName ChildName = SkelComp.GetBoneName(ChildIndex);
	FName ParentName = SkelComp.GetParentBone(ChildName);

	if (ParentName == NAME_None)
	{
		return;
	}

	int32 ParentIndex = SkelComp.GetBoneIndex(ParentName);
	FCompactPoseBoneIndex ParentBone(ParentIndex);

	FVector ChildLocation = FAnimUtil::GetBoneWorldLocation(SkelComp, Pose, ChildBone);
	FVector ParentLocation = FAnimUtil::GetBoneWorldLocation(SkelComp, Pose, ParentBone);

	DrawLine(World, ParentLocation, ChildLocation, Color, Duration, Thickness);
}

void FDebugDrawUtil::DrawBoneChain(UWorld* World, USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& Pose,
	const FCompactPoseBoneIndex& ChainStartChild, const FCompactPoseBoneIndex& ChainEndParent,
	const FLinearColor& Color, float Duration, float Thickness)
{
	int32 ParentIndex = ChainEndParent.GetInt();
	int32 ChildIndex = ChainStartChild.GetInt();

	const FName ChildName = SkelComp.GetBoneName(ChildIndex);
	const FName ParentName = SkelComp.GetBoneName(ParentIndex);

	if (!SkelComp.BoneIsChildOf(ChildName, ParentName))
	{
		return;
	}

	FName CurrChild = ChildName;

	while (CurrChild != ParentName && CurrChild != NAME_None) 
	{
		FCompactPoseBoneIndex CurrBone(SkelComp.GetBoneIndex(CurrChild));
		DrawBone(World, SkelComp, Pose, CurrBone, Color, Duration, Thickness);
		CurrChild = SkelComp.GetParentBone(CurrChild);
	}
}
