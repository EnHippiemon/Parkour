// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MyCharacterMovementDataAsset.generated.h"

UCLASS()
class PARKOUR_API UMyCharacterMovementDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="test")
	FString TestText = "Test 1";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="test")
	float FloatTest = 0.1f;

	
};
