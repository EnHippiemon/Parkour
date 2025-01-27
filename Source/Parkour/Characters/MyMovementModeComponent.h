// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyMovementModeComponent.generated.h"

class AMyCharacter;

UENUM(BlueprintType)
enum ECurrentAnimation
{
	Eca_Idle,
	Eca_Walking,
	Eca_Sprinting,
	Eca_Climbing,
	Eca_LedgeClimbing,
	Eca_Jumping,
	Eca_ClimbJumping,
	Eca_RunningUpWall,
	Eca_WallJumping,
	Eca_Aiming,
	Eca_LeavingAim,
	Eca_Exhausted,
	Eca_SlidingDown,
	Eca_Falling
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNewMovement);

UCLASS(Blueprintable, BlueprintType, ClassGroup=(My), meta=(BlueprintSpawnableComponent))
class PARKOUR_API UMyMovementModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMyMovementModeComponent();
	
	UPROPERTY(BlueprintAssignable)
	FOnNewMovement OnNewMovement;

	UTexture2D* GetCurrentMovementTexture() const { return CurrentMovementTexture; }

	void SetCurrentAnimation(ECurrentAnimation Movement);
	
private:
	// Movement mode, checks all types of movement and is used for anims etc
	TEnumAsByte<ECurrentAnimation> MovementMode;

	UPROPERTY()
	AMyCharacter* Player;

	UPROPERTY()
	USkeletalMeshComponent* Mesh;

#pragma region --- Animation Sequences ---
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* IdleAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* WalkAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* SprintAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* ClimbingAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* LedgeAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* RunUpWallAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* ClimbJumpAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* WallJumpAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* JumpAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* AimingAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* LeaveAimingAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* ExhaustedAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* SlidingDownAnim;
	UPROPERTY(EditDefaultsOnly, Category=AnimationSequence)
	UAnimSequence* FallingAnim;
#pragma endregion 

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
	UPROPERTY(EditDefaultsOnly, Category=UI)
	UTexture2D* SlidingDownTexture;
#pragma endregion

	virtual void BeginPlay() override;
};