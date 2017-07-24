// Copyright (c) Henry Cooney 2017

#include "DebugDrawUtil.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Async.h"

void FDebugDrawUtil::DrawLine(UWorld * World, const FVector & Start, const FVector & Finish, const FLinearColor & Color, float Duration, float Thickness)
{
	AsyncTask(ENamedThreads::GameThread, [World, Start, Finish, Color, Duration, Thickness]() {
		UKismetSystemLibrary::DrawDebugLine(Cast<UObject>(World), Start, Finish, Color, Duration, Thickness);
	});
}

void FDebugDrawUtil::DrawSphere(UWorld * World, const FVector & Center, const FLinearColor & Color, float Radius, int32 Segments, float Duration, float Thickness)
{ 
	AsyncTask(ENamedThreads::GameThread, [World, Center, Radius, Segments, Color, Duration, Thickness]() {
		UKismetSystemLibrary::DrawDebugSphere(Cast<UObject>(World), Center, Radius, Segments, Color, Duration, Thickness);
	});
}

void FDebugDrawUtil::DrawString(UWorld * World, const FVector & Location, const FString & Text, AActor * BaseActor, const FColor & Color, float Duration)
{
	AsyncTask(ENamedThreads::GameThread, [World, Location, Text, BaseActor, Color, Duration]() {
		UKismetSystemLibrary::DrawDebugString(Cast<UObject>(World), Location, Text, BaseActor, Color, Duration);
	});
}


