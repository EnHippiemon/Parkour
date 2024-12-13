// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnergyDataAsset.generated.h"

UCLASS()
class PARKOUR_API UEnergyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/* Energy */
		UPROPERTY(EditDefaultsOnly, Category=Energy)
		float AimEnergyDepletionSpeed = 5.f;
		UPROPERTY(EditDefaultsOnly, Category=Energy)
		float ExhaustionSpeed = 0.7f;
		UPROPERTY(EditDefaultsOnly, Category=Energy)
		float EnergyRegainSpeed = 0.4f;
		// The length the floor trace must be before losing energy
		UPROPERTY(EditDefaultsOnly, Category=Energy)
		float FloorAngleThreshold = 0.6f;
};
