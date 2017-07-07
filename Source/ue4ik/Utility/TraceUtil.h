// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "TraceUtil.generated.h"

UCLASS()
class UE4IK_API UTraceUtil : public UBlueprintFunctionLibrary
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
		bool ReturnPhysMat = false) 
	{ 
		FCollisionQueryParams TraceParams(FName(TEXT("Line Trace")), true, ActorToIgnore);
		TraceParams.bTraceComplex = true;
		//TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = ReturnPhysMat;
		
		//Ignore Actors
		TraceParams.AddIgnoredActor(ActorToIgnore);
		
		//Re-initialize hit info
		HitOut = FHitResult(ForceInit);
		
		//Trace!
		World->LineTraceSingleByChannel(
			HitOut,		//result
			Start,	//start
			End, //end
			CollisionChannel, //collision channel
			TraceParams
		);
 
		//Hit any Actor?
		return (HitOut.GetActor() != NULL);
	}
};
