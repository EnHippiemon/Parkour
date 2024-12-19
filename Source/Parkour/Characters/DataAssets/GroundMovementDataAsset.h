#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GroundMovementDataAsset.generated.h"

UCLASS()
class PARKOUR_API UGroundMovementDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/* Character Movement */
		/* Speed */
			UPROPERTY(EditDefaultsOnly, Category=MovementSpeed)
			float ReachTargetUpSpeed = 1.f;
			UPROPERTY(EditDefaultsOnly, Category=MovementSpeed)
			float ReachTargetDownSpeed = 3.f;
			UPROPERTY(EditDefaultsOnly, Category=Sprinting)
			float MaxWalkSpeed = 600.f;
			UPROPERTY(EditDefaultsOnly, Category="Sprinting|SlowingDown")
			float ThresholdToStopOverTime = 450.f;
			UPROPERTY(EditDefaultsOnly, Category="Camera|CameraPositionSpeed")
			float StandardRotationRate = 500.f;
			UPROPERTY(EditDefaultsOnly, Category="Camera|CameraPositionSpeed")
			float AimRotationRate = 30000.f;

		/* Jumping */
			UPROPERTY(EditDefaultsOnly, Category=Jump)
			float RegularJumpForce = 80000.f;
			UPROPERTY(EditDefaultsOnly, Category=Jump)
			float WallJumpUpVelocity = 900.f;
			UPROPERTY(EditDefaultsOnly, Category=Jump)
			float WallJumpBackVelocity = 400.f;
			UPROPERTY(EditDefaultsOnly, Category=Jump)
			float ThresholdToJumpBack = 0.77f;

		/* Falling */
			UPROPERTY(EditDefaultsOnly, Category=Falling)
			float GravityScale = 2.f;

		/* Sprinting */
			UPROPERTY(EditDefaultsOnly, Category=Sprinting)
			TEnumAsByte<ETraceTypeQuery> ObstacleTraceType;
			UPROPERTY(EditDefaultsOnly, Category=Sprinting)
			float DistanceBeforeAbleToRunUpWall = 500.f;
			UPROPERTY(EditDefaultsOnly, Category=Sprinting)
			float RunningUpWallTimeInSeconds = 1.f;
			UPROPERTY(EditDefaultsOnly, Category=Sprinting)
			float DistanceToRunUpWall = 75.f;
			UPROPERTY(EditDefaultsOnly, Category=Sprinting)
			float MaxSprintSpeed = 1000.f;

		/* Idle */
			UPROPERTY(EditDefaultsOnly, Category=Idle)
			float TimeBeforeIdle = 3.f;
};
