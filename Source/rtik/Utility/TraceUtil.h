// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TraceUtil.generated.h"

UCLASS()
class UTraceUtil : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	// Do a line trace from Source to Target. Code courtesy of Rama:
	// https://wiki.unrealengine.com/Trace_Functions
	UFUNCTION(BlueprintCallable, Category = Trace)
	static bool LineTrace(
		UWorld* World,
		AActor* ActorToIgnore,
		const FVector& Start,
		const FVector& End,
		FHitResult& HitOut,
		ECollisionChannel CollisionChannel = ECC_Pawn,
		bool ReturnPhysMat = false,
		bool bEnableDebugDraw = false);
};
