// Copyright (c) Henry Cooney 2017

#pragma once

#include "Runtime/Core/Public/Math/Color.h"
#include "Runtime/Core/Public/Math/Vector.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
/*
* Thread-safe debug drawing utilities. Animgraph code may be multithreaded; hence the need for locks.
*/

namespace DebugDrawUtil
{
	void DrawLine(UWorld* World, const FVector& Start, const FVector& Finish,
		const FLinearColor& Color = FColor(0, 255, 0), float Duration = -1.0f,
		float Thickness = 1.5f);

	void DrawSphere(UWorld* World, const FVector& Center, const FLinearColor& Color = FColor(0, 255, 0),
		float Radius = 15.0f, int32 Segments = 12, float Duration = -1.0f, float Thickness = 1.0f);

	void DrawString(UWorld* World, const FVector& Location, const FString& Text,
		AActor* BaseActor, const FColor& Color, float Duration = 0.0f);
}