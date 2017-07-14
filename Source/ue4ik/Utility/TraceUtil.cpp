// Copyright (c) Henry Cooney 2017

#include "Utility/TraceUtil.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "Utility/DebugDrawUtil.h"
#endif // WITH_EDITOR


bool UTraceUtil::LineTrace(
	UWorld* World,
	AActor* ActorToIgnore,
	const FVector& Start,
	const FVector& End,
	FHitResult& HitOut,
	ECollisionChannel CollisionChannel,
	bool ReturnPhysMat,
	bool bEnableDebugDraw) 
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

	bool bHitActor = (HitOut.GetActor() != nullptr);

#if WITH_EDITOR
	if (bEnableDebugDraw)
	{		
		FDebugDrawUtil::DrawLine(World, Start, End, FColor(0, 255, 255));
		if (bHitActor)
		{
			FVector HitLocation = HitOut.ImpactPoint;
			FDebugDrawUtil::DrawSphere(World, HitOut.ImpactPoint, FColor(255, 0, 0), 5.0f);	
		}
	}
#endif // WITH_EDITOR

	return bHitActor;
}
