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

	UMyCameraComponent();

	EPlayerState CurrentState;
	


	/* Camera field of view (FOV) */ 
	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraFieldOfView")
	float StillFOV = 60.f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraFieldOfView")
	float WalkingFOV = 70.f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraFieldOfView")
	float SprintingFOV = 125.f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraFieldOfView")
	float SprintFOVSpeed = 0.3f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraFieldOfView")
	float AimingFOV = 70.f;

	// void CameraMovementOutput();
	void SetCurrentCameraOffset(float& Value, float Speed, float Clamp) const;

	UFUNCTION()
	void StateSwitch(EPlayerState State);

	UPROPERTY(VisibleAnywhere)
	AMyCharacter* Player;
	
	void TickStateSwitch();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
