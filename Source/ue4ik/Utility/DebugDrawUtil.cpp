// Copyright (c) Henry Cooney 2017

#include "DebugDrawUtil.h"
#include "Kismet/KismetSystemLibrary.h"

#if PLATFORM_USE_PTHREADS
FPThreadsCriticalSection FDebugDrawUtil::CriticalSection;
#else
FCriticalSection FDebugDrawUtil::CriticalSection;
#endif

void FDebugDrawUtil::DrawLine(UWorld * World, const FVector & Start, const FVector & Finish, const FLinearColor & Color, float Duration, float Thickness)
{
	CriticalSection.Lock();
	UKismetSystemLibrary::DrawDebugLine(Cast<UObject>(World), Start, Finish, Color, Duration, Thickness);
	CriticalSection.Unlock();
}

void FDebugDrawUtil::DrawSphere(UWorld * World, const FVector & Center, const FLinearColor & Color, float Radius, int32 Segments, float Duration, float Thickness)
{
	CriticalSection.Lock();
	UKismetSystemLibrary::DrawDebugSphere(Cast<UObject>(World), Center, Radius, Segments, Color, Duration, Thickness);
	CriticalSection.Unlock();	
}

void FDebugDrawUtil::DrawString(UWorld * World, const FVector & Location, const FString & Text, AActor * BaseActor, const FColor & Color, float Duration)
{
	CriticalSection.Lock();
	UKismetSystemLibrary::DrawDebugString(Cast<UObject>(World), Location, Text, BaseActor, Color, Duration);
	CriticalSection.Unlock();
}


