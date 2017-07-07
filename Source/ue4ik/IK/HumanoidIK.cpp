// Copyright (c) Henry Cooney 2017

#include "HumanoidIK.h"
#include "Utility/AnimUtil.h"

void UHumanoidIKLibrary::HumanoidIKLegTrace(ACharacter* Character, 
	const FFabrikHumanoidLegChain& LegChain,
	FHumanoidIKTraceData& OutTraceData)
{
	// Traces to find floor points below foot bone and toe. Traces from character capsule midpoint
    // to the max step-down height.

    // Trace direction is downward axis of the character.

	FVector TraceDirection = -1 * Character->GetActorUpVector();
	UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent();
	//FVector FootLocation =



	//FVector FootTraceStart =

}
