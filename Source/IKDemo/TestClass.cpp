// Copyright (c) Henry Cooney 2017 

#include "TestClass.h"


// Sets default values
ATestClass::ATestClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATestClass::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATestClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

