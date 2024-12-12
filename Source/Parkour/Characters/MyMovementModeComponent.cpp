// Fill out your copyright notice in the Description page of Project Settings.


#include "../Characters/MyMovementModeComponent.h"

UMyMovementModeComponent::UMyMovementModeComponent()
{
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
	case Ecmm_SlidingDown:
		CurrentMovementTexture = SlidingDownTexture;
		break;
	default:
		CurrentMovementTexture = WalkingTexture;
	}
	OnNewMovement.Broadcast();
}