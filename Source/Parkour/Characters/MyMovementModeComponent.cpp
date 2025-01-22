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
		case Ecmm_Idle:
			Mesh->PlayAnimation(IdleAnim, true);
			CurrentMovementTexture = IdleTexture;
			break;
		case Ecmm_Walking:
			Mesh->PlayAnimation(WalkAnim, true);
			CurrentMovementTexture = WalkingTexture;
			break;
		case Ecmm_Sprinting:
			Mesh->PlayAnimation(SprintAnim, true);
			CurrentMovementTexture = RunningTexture;
			break;
		case Ecmm_Climbing:
			Mesh->PlayAnimation(ClimbingAnim, true);
			CurrentMovementTexture = ClimbingTexture;
			break;
		case Ecmm_LedgeClimbing:
			Mesh->PlayAnimation(LedgeAnim, false);
			CurrentMovementTexture = LedgeClimbingTexture;
			break;
		case Ecmm_Jumping:
			Mesh->PlayAnimation(JumpAnim, false);
			CurrentMovementTexture = JumpTexture;
			break;
		case Ecmm_ClimbJumping:
			Mesh->PlayAnimation(ClimbJumpAnim, false);
			CurrentMovementTexture = ClimbJumpTexture;
			break;
		case Ecmm_RunningUpWall:
			Mesh->PlayAnimation(RunUpWallAnim, true);
			CurrentMovementTexture = RunUpWallTexture;
			break;
		case Ecmm_WallJumping:
			Mesh->PlayAnimation(WallJumpAnim, true);
			CurrentMovementTexture = WallJumpTexture;
			break;
		case Ecmm_Aiming:
			Mesh->PlayAnimation(AimingAnim, true);
			CurrentMovementTexture = AimingTexture;
			break;
		case Ecmm_LeavingAim:
			Mesh->PlayAnimation(LeaveAimingAnim, true);
			CurrentMovementTexture = LeaveAimingTexture;
			break;
		case Ecmm_Exhausted:
			Mesh->PlayAnimation(ExhaustedAnim, true);
			CurrentMovementTexture = ExhaustedTexture;
			break;
		case Ecmm_SlidingDown:
			Mesh->PlayAnimation(SlidingDownAnim, true);
			CurrentMovementTexture = SlidingDownTexture;
			break;
		case Ecmm_Falling:
			Mesh->PlayAnimation(FallingAnim, true);
			break;
		default:
			Mesh->PlayAnimation(IdleAnim, true);
			CurrentMovementTexture = WalkingTexture;
		}
	}
	
	OnNewMovement.Broadcast(Movement);
}

void UMyMovementModeComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (!IsValid(Player))
		UE_LOG(LogTemp, Error, TEXT("MyMovementModeComponent.cpp - No player!"));

	Mesh = Player->FindComponentByClass<USkeletalMeshComponent>();
}

// void UMyMovementModeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
// 	FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
// 	if (MovementMode == Ecmm_Walking)
// 	{
// 		const auto PlayerSpeed = Player->GetCharacterMovement()->MaxWalkSpeed;
// 		Mesh->PlayAnimation(PlayerSpeed > 0 ? WalkAnim : IdleAnim, true);
// 	}
// }
