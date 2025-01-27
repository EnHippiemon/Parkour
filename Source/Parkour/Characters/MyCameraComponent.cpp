// Fill out your copyright notice in the Description page of Project Settings.


#include "../Characters/MyCameraComponent.h"
#include "../Characters/MyCharacter.h"
#include "DataAssets/CameraDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UMyCameraComponent::UMyCameraComponent()
{
	UMyCameraComponent::SetAutoActivate(true);

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	bUsePawnControlRotation = false;

	static ConstructorHelpers::FObjectFinder<UCameraDataAsset> CameraDataAsset(TEXT("/Game/Player/DataAssets/CameraDataAsset"));
	if (CameraDataAsset.Object)
		CameraData = CameraDataAsset.Object;
}

void UMyCameraComponent::StateSwitch(EPlayerState State)
{
	CurrentState = State;
}

void UMyCameraComponent::TickStateSwitch()
{
	const auto DeltaTime = GetWorld()->DeltaTimeSeconds;
	switch(CurrentState)
	{
	case Eps_Walking:
		FieldOfView = Player->GetCharacterMovement()->Velocity.Length() == 0
		? FieldOfView = FMath::Lerp(FieldOfView, CameraData->StillFOV, CameraData->StillFOVSpeed * DeltaTime)
		: FieldOfView = FMath::Lerp(FieldOfView, CameraData->WalkingFOV, CameraData->WalkingFOVSpeed * DeltaTime);
		break;
	case Eps_Sprinting:
		FieldOfView = FMath::Lerp(FieldOfView, CameraData->SprintingFOV, CameraData->SprintFOVSpeed * DeltaTime);
		break;
	case Eps_Idle:
		FieldOfView = FMath::Lerp(FieldOfView, CameraData->IdleFOV, CameraData->IdleFOVSpeed * DeltaTime);
		break;
	case Eps_Aiming:
		FieldOfView = FMath::Lerp(FieldOfView, CameraData->AimingFOV, CameraData->AimingFOVSpeed * DeltaTime);
		break;
	case Eps_LeaveAiming:
		FieldOfView = FMath::Lerp(FieldOfView, CameraData->WalkingFOV, CameraData->WalkingFOVSpeed * DeltaTime);
		break;
	case Eps_Climbing:
		FieldOfView = FMath::Lerp(FieldOfView, CameraData->WalkingFOV, CameraData->ClimbingFOVSpeed * DeltaTime);
		break;
	default:
		break;
	}
}

void UMyCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (IsValid(Player))
		Player->OnStateChanged.AddUniqueDynamic(this, &UMyCameraComponent::StateSwitch);

	if (!CameraData)
		UE_LOG(LogTemp, Error, TEXT("MyCameraComponent.cpp - Data asset missing!"));
}

void UMyCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(Player))
		return;
	
	TickStateSwitch();
}