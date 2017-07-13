// Copyright (c) Henry Cooney 2017

#include "DebugDrawUtil.h"
#include "Kismet/KismetSystemLibrary.h"

#if PLATFORM_USE_PTHREADS
#include "HAL/PThreadCriticalSection.h"
FPThreadsCriticalSection CriticalSection;
#else
#include "Windows/WindowsPlatformProcess.h"
FCriticalSection CriticalSection;
#endif

namespace DebugDrawUtil
{

	void DrawLine(UWorld * World, const FVector & Start, const FVector & Finish, const FLinearColor & Color, float Duration, float Thickness)
	{
		CriticalSection.Lock();
		UKismetSystemLibrary::DrawDebugLine(World, Start, Finish, Color, Duration, Thickness);
		CriticalSection.Unlock();
	}

	void DrawSphere(UWorld * World, const FVector & Center, const FLinearColor & Color, float Radius, int32 Segments, float Duration, float Thickness)
	{
		CriticalSection.Lock();
		UKismetSystemLibrary::DrawDebugSphere(World, Center, Radius, Segments, Color, Duration, Thickness);
		CriticalSection.Unlock();

	}

	void DrawString(UWorld * World, const FVector & Location, const FString & Text, AActor * BaseActor, const FColor & Color, float Duration)
	{
		CriticalSection.Lock();
		UKismetSystemLibrary::DrawDebugString(World, Location, Text, BaseActor, Color, Duration);
		CriticalSection.Unlock();
	}

}

