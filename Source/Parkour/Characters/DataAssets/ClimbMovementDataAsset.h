// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ClimbMovementDataAsset.generated.h"

UCLASS()
class PARKOUR_API UClimbMovementDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/* Climbing */
		UPROPERTY(EditDefaultsOnly, Category=Climbing)
		TEnumAsByte<ECollisionChannel> ClimbingCollision;
		// Distance to check wall surface normal for rotation correction
		UPROPERTY(EditDefaultsOnly, Category=Climbing)
		float PlayerToWallDistance = 70.f;
		UPROPERTY(EditDefaultsOnly, Category=Climbing)
		float ClimbingWidth = 40.f;
		UPROPERTY(EditDefaultsOnly, Category=Climbing)
		float RotateToWallSpeed = 0.5f;

		UPROPERTY(EditDefaultsOnly, Category="Climbing|Jump")
		float ClimbJumpingTime = 0.5f;
		
		/* Ledge climbing */
			UPROPERTY (EditDefaultsOnly, Category="Climbing|Ledge")
			float BottomLedgeDetectionZOffset = 40.f;
			UPROPERTY (EditDefaultsOnly, Category="Climbing|Ledge")
			FVector LedgeClimbDetectionOffset = FVector(60.f, 0.f, 70.f);
			UPROPERTY(EditDefaultsOnly, Category="Climbing|Ledge")
			float LedgeClimbDuration = 1.f;
			UPROPERTY(EditDefaultsOnly, Category="Climbing|Ledge")
			TEnumAsByte<ECollisionChannel> LedgeChannel;
};
