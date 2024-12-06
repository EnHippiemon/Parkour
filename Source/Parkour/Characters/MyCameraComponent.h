// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Characters/MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "MyCameraComponent.generated.h"

enum EPlayerState;

UCLASS(Blueprintable, ClassGroup=(My), meta=(BlueprintSpawnableComponent))
class PARKOUR_API UMyCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

private:
	UMyCameraComponent();

#pragma region ---------- Variables ----------
	/* Camera field of view (FOV) */ 
	UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
	float IdleFOV = 60.f;
	UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
	float StillFOV = 60.f;
	UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
	float WalkingFOV = 70.f;
	UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
	float SprintingFOV = 125.f;
	UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
	float AimingFOV = 70.f;
	UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView")
	float ClimbingFOV = 70.f;

	/* Camera transition speed field of view (FOV) */ 
	UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
	float IdleFOVSpeed = 0.0001f;
	UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
	float StillFOVSpeed = 0.02f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
	float WalkingFOVSpeed = 0.005f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
	float SprintFOVSpeed = 0.01f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
	float AimingFOVSpeed = 0.3f;
		UPROPERTY(EditDefaultsOnly, Category="CameraFieldOfView|Speed")
	float ClimbingFOVSpeed = 0.02f;

#pragma endregion
	
	UFUNCTION()
	void StateSwitch(EPlayerState State);

	UPROPERTY(VisibleAnywhere)
	AMyCharacter* Player;
	
	void TickStateSwitch();

	EPlayerState CurrentState;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
