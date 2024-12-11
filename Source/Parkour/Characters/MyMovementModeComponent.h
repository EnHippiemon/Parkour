// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyMovementModeComponent.generated.h"

enum ECurrentMovementMode
{
	Ecmm_Idle,
	Ecmm_Walking,
	Ecmm_Sprinting,
	Ecmm_Climbing,
	Ecmm_LedgeClimbing,
	Ecmm_Jumping,
	Ecmm_ClimbJumping,
	Ecmm_RunningUpWall,
	Ecmm_WallJumping,
	Ecmm_Aiming,
	Ecmm_LeavingAim,
	Ecmm_Exhausted
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNewMovement);

UCLASS(Blueprintable, BlueprintType, ClassGroup=(My), meta=(BlueprintSpawnableComponent))
class PARKOUR_API UMyMovementModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMyMovementModeComponent();
	
	UPROPERTY(BlueprintAssignable)
	FOnNewMovement OnNewMovement;

	UTexture2D* GetCurrentMovementTexture() { return CurrentMovementTexture; }

	void SetCurrentMovementMode(ECurrentMovementMode Movement);
	
private:
	// Movement mode, checks all types of movement and is used for anims etc
	TEnumAsByte<ECurrentMovementMode> MovementMode;

#pragma region --- UI Textures ---
	UPROPERTY()
	UTexture2D* CurrentMovementTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* IdleTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* WalkingTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* RunningTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* ClimbingTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* LedgeClimbingTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* RunUpWallTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* ClimbJumpTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* WallJumpTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* JumpTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* AimingTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* LeaveAimingTexture;
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* ExhaustedTexture;
#pragma endregion

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
