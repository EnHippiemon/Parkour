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
	UPROPERTY(EditDefaultsOnly, Category=Energy)
	float AimEnergyDepletionSpeed = 5.f;
	UPROPERTY(EditDefaultsOnly, Category=Energy)
	float RunningDepletionSpeed = 0.7f;
	UPROPERTY(EditDefaultsOnly, Category="Energy|Climbing")
	float ClimbingDepletionSpeed = 0.2f;
	UPROPERTY(EditDefaultsOnly, Category="Energy|Climbing")
	float EnergyBoostStartAmount = 0.1f;
	UPROPERTY(EditDefaultsOnly, Category="Energy|Climbing")
	float AmountOfWallPitchToCount = 0.025f;
	UPROPERTY(EditDefaultsOnly, Category=Energy)
	float EnergyRegainSpeed = 0.4f;
	// The length the floor trace must be before losing energy
	UPROPERTY(EditDefaultsOnly, Category=Energy)
	float FloorAngleThreshold = 0.6f;
	UPROPERTY(EditDefaultsOnly, Category=Energy)
	float JumpEnergyLoss = 0.3f;
};