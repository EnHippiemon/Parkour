// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpringArmDataAsset.generated.h"

UCLASS()
class PARKOUR_API USpringArmDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/* Rotation */
		UPROPERTY(EditDefaultsOnly, Category="Rotation")
		float StandardRotationSpeed = 3000.f;
		UPROPERTY(EditDefaultsOnly, Category="Rotation")
		float AimingRotationSpeed = 500000.f;
		
	/* Offset */
		/* Spring arm socket offset */
			UPROPERTY(EditDefaultsOnly, Category="Offset|Socket")
			FVector ClimbingSpringArmTargetOffset = FVector(-70, 0, 0);;
			UPROPERTY(EditDefaultsOnly, Category="Offset|Socket")
			FVector OffsetClamp = FVector(0.f, 100.f, 80.f);

		/* Spring arm target offset */
			// Decides how far to the sides the camera can move.
			UPROPERTY(EditDefaultsOnly, Category="Offset|Target")
			FVector AimingCameraOffset = FVector(0.f, 80.f, 0.f);

		/* Offset speed */
			// Decides how quickly the camera moves from side to side. 
			UPROPERTY(EditDefaultsOnly, Category="Offset|Speed")
			float CameraYDirectionSpeed = 1000.f;
			UPROPERTY(EditDefaultsOnly, Category="Offset|Speed")
			float ClimbingOffsetSpeed = 0.01f;
			UPROPERTY(EditDefaultsOnly, Category="Offset|Speed")
			float AimingOffsetSpeed = 0.2f;
	
	/* Spring arm length */
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float WalkingSpringArmLength = 400.f;
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float SprintingSpringArmLength = 400.f;
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float HookshotSpringArmLength = 350.f;
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float IdleSpringArmLength = 5000.f;
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float AimingSpringArmLength = 120.f;
		UPROPERTY(EditDefaultsOnly, Category="SpringArmLength")
		float ClimbingSpringArmLength = 400.f;


	/* Spring arm extension speed */ 
		UPROPERTY(EditDefaultsOnly, Category="ExtensionSpeed")
		float SprintExtensionSpeed = 0.01f;
		UPROPERTY(EditDefaultsOnly, Category="ExtensionSpeed")
		float WalkingExtensionSpeed = 0.02f;
		UPROPERTY(EditDefaultsOnly, Category="ExtensionSpeed")
		float IdleExtensionSpeed = 0.00005f;
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

	/* Collision */
		UPROPERTY(EditDefaultsOnly, Category=Energy)
		TEnumAsByte<ECollisionChannel> BlockAllCollision;
};
