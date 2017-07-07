// Copyright (c) Henry Cooney 2017


#include "DebugDrawUtil.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/AnimUtil.h"
#include "Components/SkeletalMeshComponent.h"

#if PLATFORM_USE_PTHREADS
#include "HAL/PThreadCriticalSection.h"
FPThreadsCriticalSection CriticalSection;
#else
#include "Windows/WindowsPlatformProcess.h"
FCriticalSection CriticalSection;
#endif

void DebugDrawUtil::DrawLine(UObject* World, const FVector& Start, const FVector& Finish, const FLinearColor& Color, float Duration, float Thickness)
{
	CriticalSection.Lock();
	UKismetSystemLibrary::DrawDebugLine(World, Start, Finish, Color, Duration, Thickness);
	CriticalSection.Unlock();
}

void DebugDrawUtil::DrawSphere(UObject* World, const FVector& Center, const FLinearColor& Color, float Radius, int32 Segments, float Duration, float Thickness)
{
	CriticalSection.Lock();
	UKismetSystemLibrary::DrawDebugSphere(World, Center, Radius, Segments, Color, Duration, Thickness);
	CriticalSection.Unlock();
}

void DebugDrawUtil::DrawString(UObject* World, const FVector& Location, const FString& Text, AActor* BaseActor,
	const FColor& Color, float Duration)
{
	CriticalSection.Lock();
	UKismetSystemLibrary::DrawDebugString(World, Location, Text, BaseActor, Color, Duration);
	CriticalSection.Unlock();
}
