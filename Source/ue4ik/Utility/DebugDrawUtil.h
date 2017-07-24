// Copyright (c) Henry Cooney 2017

#pragma once

#include "Runtime/Core/Public/Math/Color.h"
#include "Runtime/Core/Public/Math/Vector.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Runtime/Engine/Public/BonePose.h"
#include "DebugDrawUtil.generated.h"
/*
* Thread-safe debug drawing utilities. Animgraph code may be multithreaded; debug-drawing on animation threads 
* seems to cause crashes. Functions in this class use AsyncTasks to draw on the game thread, instead. 
*/

USTRUCT()
struct UE4IK_API FDebugDrawUtil
{
	GENERATED_USTRUCT_BODY()

public: 

	static void DrawLine(UWorld* World, const FVector& Start, const FVector& Finish,
		const FLinearColor& Color= FColor(0, 255, 0), float Duration = -1.0f, float Thickness = 1.5f);

	static void DrawSphere(UWorld* World, const FVector& Center, const FLinearColor& Color = FColor(0, 255, 0),
		float Radius = 15.0f, int32 Segments = 12, float Duration = -1.0f, float Thickness = 1.0f);

	static void DrawString(UWorld* World, const FVector& Location, const FString& Text,
		AActor* BaseActor, const FColor& Color, float Duration = 0.0f);

	static void DrawPlane(UWorld * World, const FVector& PlaneBase, const FVector& PlaneNormal, float Size = 100.0f,
		const FLinearColor& Color = FColor(255, 0, 255), bool bDrawNormal = true, float Duration = -1.0f);

	// Draws a direction vector, starting at base, with some pre-decided length. Set length to a negative
	static void DrawVector(UWorld* World, const FVector& Base, FVector Direction,
		const FLinearColor& Color = FColor(255, 0, 0), float Length = 50.0f, 
		float Duration = -1.0f, float Thickness = 1.5f);

	// Draw a line from bone to its skeletal parent
	static void DrawBone(UWorld * World, USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& Pose,
		const FCompactPoseBoneIndex& ChildBone, const FLinearColor& Color = FColor(0, 255, 255), 
		float Duration = -1.0f, float Thickness = 1.5f);

	// Draw bones from ChainStartChild to ChainEndParent. ChainStartChild must be a parent of ChainEndParent,
	// otherwise nothing will happen.
	static void DrawBoneChain(UWorld* World, USkeletalMeshComponent& SkelComp, FCSPose<FCompactPose>& Pose,
		const FCompactPoseBoneIndex & ChainStartChild, const FCompactPoseBoneIndex& ChainEndParent,
		const FLinearColor& Color = FColor(0, 255, 255), float Duration = -1.0f, float Thickness = 1.5f);
};