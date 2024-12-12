// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraDataAsset.generated.h"

UCLASS()
class PARKOUR_API UCameraDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/* Camera field of view (FOV) */ 
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
		float IdleFOV = 60.f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
		float StillFOV = 70.f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
		float WalkingFOV = 80.f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
		float SprintingFOV = 90.f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
		float AimingFOV = 80.f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
		float ClimbingFOV = 80.f;

	/* Camera transition speed field of view (FOV) */ 
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
		float IdleFOVSpeed = 0.0001f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
		float StillFOVSpeed = 0.002f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
		float WalkingFOVSpeed = 0.005f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
		float SprintFOVSpeed = 0.01f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
		float AimingFOVSpeed = 0.3f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
		float ClimbingFOVSpeed = 0.02f;
};