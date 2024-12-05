// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Characters/MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "MySpringArmComponent.generated.h"

enum EPlayerState;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(My), meta=(BlueprintSpawnableComponent))
class PARKOUR_API UMySpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

	UMySpringArmComponent();

private:
	/* Camera */
	/* Camera speed */
	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraSpeed")
	float StandardCameraSpeed = 5000.f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraSpeed")
	float AimCameraSpeed = 50000.f;
	
	// Decides how quickly the camera moves from side to side. 
	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraPositionSpeed")
	float CameraYDirectionSpeed = 1000.f;
		
	float CurrentCameraSpeed;
	/* Spring Arm */
	/* Spring arm length */
	UPROPERTY(EditDefaultsOnly, Category="Camera|SpringArm|SpringArmLength")
	float StandardSpringArmLength = 350.f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|SpringArm|SpringArmLength")
	float SprintingSpringArmLength = 350.f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|SpringArm|SpringArmLength")
	float StopAimingSpringArmLength = 300.f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|SpringArm|SpringArmLength")
	float TraceLengthWallBehindPlayer = 275.f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|SpringArm|SpringArmLength")
	float ArmLengthWallBehindPlayer = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|SpringArm|SpringArmLength")
	float ResetTimeWallBehindPlayer = 1.f;

	float TimeSinceWallBehindPlayer = 0.f;
	
	/* Spring arm extension speed */ 
	UPROPERTY(EditDefaultsOnly, Category="Camera|SpringArm|SpringExtensionSpeed")
	float SpringArmSwitchSpeed = 0.01f;
	UPROPERTY(EditDefaultsOnly, Category="Camera|SpringArm|SpringExtensionSpeed")
	float NormalCameraSwitchSpeed = 0.02f;

	/* Spring arm target offset */
	UPROPERTY(EditDefaultsOnly, Category="Camera|SpringArm")
	FVector ClimbingSpringArmTargetOffset = FVector(-70, 0, 0);;

	/* Camera position */

	// Decides how far to the sides the camera can move.
	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraPosition")
	FVector AimingCameraOffset = FVector(0.f, 50.f, -10.f);

	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraPosition")
	FVector CameraClamp = FVector(0.f, 200.f, 100.f);

	float CurrentCameraOffsetY = 150.f;
	float CurrentCameraOffsetZ = 0.f;

	/* Camera positioning speed */ 
	UPROPERTY(EditDefaultsOnly, Category="Camera|CameraPositionSpeed")
	float AimingCameraTransitionAlpha = 0.3f;


	bool bCanJumpBack = false;

	void CameraMovementOutput();
	void CalculateCameraOffset(FVector2D CameraMove);
	void SetCameraOffset();
	
	UFUNCTION()
	void StateSwitch(EPlayerState State);

	UFUNCTION()
	void UpdateCanJumpBack(bool CanJumpBack);

	void TickStateSwitch();
	void CameraLerp(float& Value, float Speed, float Clamp) const;
	void CheckWallBehindPlayer();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TEnumAsByte<EPlayerState> CurrentState;
	
	UPROPERTY(VisibleAnywhere)
	AMyCharacter* Player;
	
	UPROPERTY(EditDefaultsOnly, Category=Energy)
	TEnumAsByte<ECollisionChannel> BlockAllCollision;
};
