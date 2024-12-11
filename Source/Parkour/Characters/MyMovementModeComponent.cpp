// Fill out your copyright notice in the Description page of Project Settings.


#include "../Characters/MyMovementModeComponent.h"

// Sets default values for this component's properties
UMyMovementModeComponent::UMyMovementModeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UMyMovementModeComponent::SetCurrentMovementMode(ECurrentMovementMode Movement)
{
	if (MovementMode == Movement)
		return;
	
	MovementMode = Movement;

	switch (MovementMode)
	{
	case Ecmm_Idle:
		CurrentMovementTexture = IdleTexture;
		break;
	case Ecmm_Walking:
		CurrentMovementTexture = WalkingTexture;
		break;
	case Ecmm_Sprinting:
		CurrentMovementTexture = RunningTexture;
		break;
	case Ecmm_Climbing:
		CurrentMovementTexture = ClimbingTexture;
		break;
	case Ecmm_LedgeClimbing:
		CurrentMovementTexture = LedgeClimbingTexture;
		break;
	case Ecmm_Jumping:
		CurrentMovementTexture = JumpTexture;
		break;
	case Ecmm_ClimbJumping:
		CurrentMovementTexture = ClimbJumpTexture;
		break;
	case Ecmm_RunningUpWall:
		CurrentMovementTexture = RunUpWallTexture;
		break;
	case Ecmm_WallJumping:
		CurrentMovementTexture = WallJumpTexture;
		break;
	case Ecmm_Aiming:
		CurrentMovementTexture = AimingTexture;
		break;
	case Ecmm_LeavingAim:
		CurrentMovementTexture = LeaveAimingTexture;
		break;
	case Ecmm_Exhausted:
		CurrentMovementTexture = ExhaustedTexture;
		break;
	default:
		CurrentMovementTexture = WalkingTexture;
	}
	UE_LOG(LogTemp, Log, TEXT("Movement mode: %d"), MovementMode.GetValue())
	OnNewMovement.Broadcast();
}

// Called when the game starts
void UMyMovementModeComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UMyMovementModeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

