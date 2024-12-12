// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Characters/MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/DataAsset.h"
#include "MySpringArmComponent.generated.h"

enum EPlayerState;

class USpringArmDataAsset;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(My), meta=(BlueprintSpawnableComponent))
class PARKOUR_API UMySpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

private:
	UMySpringArmComponent();

#pragma region ------------ Variables -------------
	/* Rotation */
		float RotationSpeed;
		bool bShouldRotateFast = false;

	/* Offset */
		/* Spring arm socket offset */
			float CurrentCameraOffsetY = 150.f;
			float CurrentCameraOffsetZ = 0.f;

	/* Situational */
		/* Wall behind player */
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
	USpringArmDataAsset* ArmData;
};