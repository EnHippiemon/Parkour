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

private:
	
	UMySpringArmComponent();

#pragma region ------------ Variables -------------
	/* Rotation */
		UPROPERTY(EditDefaultsOnly, Category="Rotation")
		float StandardRotationSpeed = 5000.f;
		UPROPERTY(EditDefaultsOnly, Category="Rotation")
		float AimingRotationSpeed = 50000.f;
		
		float RotationSpeed;
		bool bShouldRotateFast = false;

	/* Offset */
		/* Spring arm socket offset */
			UPROPERTY(EditDefaultsOnly, Category="Offset|Socket")
			FVector ClimbingSpringArmTargetOffset = FVector(-70, 0, 0);;
			UPROPERTY(EditDefaultsOnly, Category="Offset|Socket")
			FVector OffsetClamp = FVector(0.f, 200.f, 100.f);

			float CurrentCameraOffsetY = 150.f;
			float CurrentCameraOffsetZ = 0.f;

		/* Spring arm target offset */
			// Decides how far to the sides the camera can move.
			UPROPERTY(EditDefaultsOnly, Category="Offset|Target")
			FVector AimingCameraOffset = FVector(0.f, 50.f, -10.f);

		/* Offset speed */
			// Decides how quickly the camera moves from side to side. 
			UPROPERTY(EditDefaultsOnly, Category="Offset|Speed")
			float CameraYDirectionSpeed = 1000.f;
			UPROPERTY(EditDefaultsOnly, Category="Offset|Speed")
			float ClimbingOffsetSpeed = 0.01f;
			UPROPERTY(EditDefaultsOnly, Category="Offset|Speed")
			float AimingOffsetSpeed = 0.3f;
	
	/* Spring arm length */
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float WalkingSpringArmLength = 350.f;
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float SprintingSpringArmLength = 350.f;
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float HookshotSpringArmLength = 300.f;
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float IdleSpringArmLength = 10000.f;
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float AimingSpringArmLength = 100.f;
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float ClimbingSpringArmLength = 350.f;


	/* Spring arm extension speed */ 
		UPROPERTY(EditDefaultsOnly, Category="ExtensionSpeed")
		float SprintExtensionSpeed = 0.01f;
		UPROPERTY(EditDefaultsOnly, Category="ExtensionSpeed")
		float WalkingExtensionSpeed = 0.02f;
		UPROPERTY(EditDefaultsOnly, Category="ExtensionSpeed")
		float IdleExtensionSpeed = 0.0001f;
		UPROPERTY(EditDefaultsOnly, Category="ExtensionSpeed")
		float AimingExtensionSpeed = 0.3f;
		UPROPERTY(EditDefaultsOnly, Category="ExtensionSpeed")
		float HookshotExtensionSpeed = 0.02f;
		UPROPERTY(EditDefaultsOnly, Category="ExtensionSpeed")
		float ClimbingExtensionSpeed = 0.02f;

	/* Situational */
		/* Wall behind player */
			UPROPERTY(EditDefaultsOnly, Category="Situational|WallBehindPlayer")
			float TraceLengthWallBehindPlayer = 275.f;
			UPROPERTY(EditDefaultsOnly, Category="Situational|WallBehindPlayer")
			float ArmLengthWallBehindPlayer = 1000.f;
			UPROPERTY(EditDefaultsOnly, Category="Situational|WallBehindPlayer")
			float ResetTimeWallBehindPlayer = 1.f;

			float TimeSinceWallBehindPlayer = 0.f;
			bool bWallIsInFront = false;

#pragma endregion
	
	void CameraMovementOutput();
	void CameraOffsetByLooking(FVector2D CameraMove);
	void CameraOffsetByMovement();
	void SetCameraOffset();
	void SetRotationSpeed();
	
	UFUNCTION()
	void StateSwitch(EPlayerState State);

	UFUNCTION()
	void UpdateWallIsInFront(bool CanJumpBack);

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
