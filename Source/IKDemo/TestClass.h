// Copyright (c) Henry Cooney 2017 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestClass.generated.h"

UCLASS()
class IKDEMO_API ATestClass : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestClass();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
