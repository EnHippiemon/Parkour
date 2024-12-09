// Fill out your copyright notice in the Description page of Project Settings.


#include "../Characters/MyCameraComponent.h"
// #include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UMyCameraComponent::UMyCameraComponent()
{
	UMyCameraComponent::SetAutoActivate(true);

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	bUsePawnControlRotation = false;
}

void UMyCameraComponent::StateSwitch(EPlayerState State)
{
	CurrentState = State;
}

void UMyCameraComponent::TickStateSwitch()
{
	if (!IsValid(Player))
	{
		UE_LOG(LogTemp, Error, TEXT("MyCameraComponent.cpp - No player found!"))
		return;
	}
	
	switch(CurrentState)
	{
	case Eps_Walking:
		FieldOfView = Player->GetCharacterMovement()->Velocity.Length() == 0
		? FieldOfView = FMath::Lerp(FieldOfView, StillFOV, StillFOVSpeed)
		: FieldOfView = FMath::Lerp(FieldOfView, WalkingFOV, WalkingFOVSpeed);
		break;
	case Eps_Sprinting:
		FieldOfView = FMath::Lerp(FieldOfView, SprintingFOV, SprintFOVSpeed);
		break;
	case Eps_Idle:
		FieldOfView = FMath::Lerp(FieldOfView, IdleFOV, IdleFOVSpeed);
		break;
	case Eps_Aiming:
		FieldOfView = FMath::Lerp(FieldOfView, AimingFOV, AimingFOVSpeed);
		break;
	case Eps_LeaveAiming:
		FieldOfView = FMath::Lerp(FieldOfView, WalkingFOV, WalkingFOVSpeed);
		break;
	case Eps_Climbing:
		FieldOfView = FMath::Lerp(FieldOfView, WalkingFOV, ClimbingFOVSpeed);
		break;
	default:
		break;
	}
	
	// if (CurrentState != Eps_Idle)
	// 	FieldOfView = FMath::Lerp(FieldOfView, WalkingFOV, 0.001f);
}

void UMyCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (IsValid(Player))
	{
		Player->OnStateChanged.AddUniqueDynamic(this, &UMyCameraComponent::StateSwitch);
	}
}

void UMyCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickStateSwitch();
}