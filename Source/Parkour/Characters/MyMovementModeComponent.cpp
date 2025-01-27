// Fill out your copyright notice in the Description page of Project Settings.


#include "../Characters/MyMovementModeComponent.h"

#include "MyCharacter.h"
#include "Kismet/GameplayStatics.h"

UMyMovementModeComponent::UMyMovementModeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMyMovementModeComponent::SetCurrentAnimation(ECurrentAnimation Movement)
{
	if (MovementMode == Movement)
		return;
	
	MovementMode = Movement;

	if (Mesh)
	{
		switch (MovementMode)
		{
		case Eca_Idle:
			Mesh->PlayAnimation(IdleAnim, true);
			CurrentMovementTexture = IdleTexture;
			break;
		case Eca_Walking:
			Mesh->PlayAnimation(WalkAnim, true);
			CurrentMovementTexture = WalkingTexture;
			break;
		case Eca_Sprinting:
			Mesh->PlayAnimation(SprintAnim, true);
			CurrentMovementTexture = RunningTexture;
			break;
		case Eca_Climbing:
			Mesh->PlayAnimation(ClimbingAnim, true);
			CurrentMovementTexture = ClimbingTexture;
			break;
		case Eca_LedgeClimbing:
			Mesh->PlayAnimation(LedgeAnim, false);
			CurrentMovementTexture = LedgeClimbingTexture;
			break;
		case Eca_Jumping:
			Mesh->PlayAnimation(JumpAnim, false);
			CurrentMovementTexture = JumpTexture;
			break;
		case Eca_ClimbJumping:
			Mesh->PlayAnimation(ClimbJumpAnim, false);
			CurrentMovementTexture = ClimbJumpTexture;
			break;
		case Eca_RunningUpWall:
			Mesh->PlayAnimation(RunUpWallAnim, true);
			CurrentMovementTexture = RunUpWallTexture;
			break;
		case Eca_WallJumping:
			Mesh->PlayAnimation(WallJumpAnim, true);
			CurrentMovementTexture = WallJumpTexture;
			break;
		case Eca_Aiming:
			Mesh->PlayAnimation(AimingAnim, true);
			CurrentMovementTexture = AimingTexture;
			break;
		case Eca_LeavingAim:
			Mesh->PlayAnimation(LeaveAimingAnim, true);
			CurrentMovementTexture = LeaveAimingTexture;
			break;
		case Eca_Exhausted:
			Mesh->PlayAnimation(ExhaustedAnim, true);
			CurrentMovementTexture = ExhaustedTexture;
			break;
		case Eca_SlidingDown:
			Mesh->PlayAnimation(SlidingDownAnim, true);
			CurrentMovementTexture = SlidingDownTexture;
			break;
		case Eca_Falling:
			Mesh->PlayAnimation(FallingAnim, true);
			CurrentMovementTexture = JumpTexture;
			break;
		default:
			Mesh->PlayAnimation(IdleAnim, true);
			CurrentMovementTexture = WalkingTexture;
		}
	}
	
	OnNewMovement.Broadcast();
}

void UMyMovementModeComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (!IsValid(Player))
		UE_LOG(LogTemp, Error, TEXT("MyMovementModeComponent.cpp - No player!"));

	Mesh = Player->FindComponentByClass<USkeletalMeshComponent>();
}