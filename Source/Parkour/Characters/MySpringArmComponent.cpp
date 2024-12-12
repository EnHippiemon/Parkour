// Fill out your copyright notice in the Description page of Project Settings.


#include "../Characters/MySpringArmComponent.h"

#include "MyCameraComponent.h"
#include "DataAssets/SpringArmDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UMySpringArmComponent::UMySpringArmComponent()
{
	UMySpringArmComponent::SetAutoActivate(true);
	
	static ConstructorHelpers::FObjectFinder<USpringArmDataAsset> SpringArmDataAsset(TEXT("/Game/Player/DataAssets/SpringArmDataAsset"));
	if (SpringArmDataAsset.Object)
		ArmData = SpringArmDataAsset.Object;

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	bUsePawnControlRotation = true;
	bEnableCameraRotationLag = true;
	CameraRotationLagSpeed = 50.f;
	CameraLagSpeed = 20.f;
	CameraLagMaxDistance = 3.f;
}

void UMySpringArmComponent::CameraMovementOutput()
{
	if (!IsValid(Player->Controller))
	{
		UE_LOG(LogTemp, Error, TEXT("MySpringArmComponent - no controller"));
		return;
	}
	
	const FVector2D CameraMove = Player->GetCameraInput();
	CameraOffsetByLooking(CameraMove);
	
	if (CameraMove == FVector2D(0, 0))
		return;
	
	// Add rotation
	Player->AddControllerYawInput(CameraMove.X * RotationSpeed * GetWorld()->DeltaTimeSeconds);
	Player->AddControllerPitchInput(-CameraMove.Y * RotationSpeed * GetWorld()->DeltaTimeSeconds);
}

// Camera offset by mouse input 
void UMySpringArmComponent::CameraOffsetByLooking(const FVector2D CameraMove)
{
	const FVector CharacterForwardVector = Player->GetActorForwardVector();
	constexpr float InputSensitivityThreshold = 0.005f;
	
	float CameraSpeed = ArmData->CameraYDirectionSpeed / (CharacterForwardVector.Length() + 1);
	if (CurrentState == Eps_Climbing)
	{
		CameraSpeed = ArmData->CameraYDirectionSpeed * 2;
		if (CameraMove.Y < -InputSensitivityThreshold)
			CameraLerp(CurrentCameraOffsetZ, -CameraSpeed, ArmData->OffsetClamp.Z);
		else if (CameraMove.Y > InputSensitivityThreshold)
			CameraLerp(CurrentCameraOffsetZ, CameraSpeed, ArmData->OffsetClamp.Z);
	}
	
	if (CameraMove.X < -InputSensitivityThreshold)
		CameraLerp(CurrentCameraOffsetY, -CameraSpeed, ArmData->OffsetClamp.Y);
	else if (CameraMove.X > InputSensitivityThreshold)
		CameraLerp(CurrentCameraOffsetY, CameraSpeed, ArmData->OffsetClamp.Y);
}

// Set camera offset in relation to character based on character movement
void UMySpringArmComponent::CameraOffsetByMovement()
{
	// Check if character is moving 
	if (Player->GetCharacterMovement()->Velocity.Length() < 0.1f)
		return;

	const auto CharacterMovement = Player->GetMovementInput();
	const auto Camera = Player->GetCamera();
	const auto CameraRightVector = Camera->GetRightVector();

	// ---------- TO DO: LOOK HERE ------------ 
	// MIGHT BE THE WRONG THRESHOLD? 1.8 AND 0.2?
	constexpr float LeftThreshold = 1.8f;
	constexpr float RightThreshold = 0.8f;

	// Sideways camera movement. Check if character's movement is towards left or right side of camera. 
	if ((CharacterMovement.Rotation().Vector() - CameraRightVector).Length() > LeftThreshold)
		CameraLerp(CurrentCameraOffsetY, -ArmData->CameraYDirectionSpeed, ArmData->OffsetClamp.Y);
	else if ((CharacterMovement.Rotation().Vector() - CameraRightVector).Length() < RightThreshold)
		CameraLerp(CurrentCameraOffsetY, ArmData->CameraYDirectionSpeed, ArmData->OffsetClamp.Y);

	if (CurrentState != Eps_Climbing)
		return;
	
	constexpr float UpThreshold = 0.5f;
	constexpr float DownThreshold = -0.5f;

	// Upwards camera movement (while climbing!)
	if (Player->GetMovementInput().Z > UpThreshold)
		CameraLerp(CurrentCameraOffsetZ, ArmData->CameraYDirectionSpeed, ArmData->OffsetClamp.Z);
	else if (Player->GetMovementInput().Z < DownThreshold)
		CameraLerp(CurrentCameraOffsetZ, -ArmData->CameraYDirectionSpeed, ArmData->OffsetClamp.Z);
}

void UMySpringArmComponent::SetCameraOffset()
{
	if (CurrentState != Eps_Climbing)
	{
		CurrentCameraOffsetZ = FMath::Lerp(CurrentCameraOffsetZ, 0.f, ArmData->WalkingExtensionSpeed);
		TargetOffset = FMath::Lerp(TargetOffset, FVector(0, 0, 0), ArmData->SprintExtensionSpeed);
	}

	if (CurrentState != Eps_Aiming)
		SocketOffset = FMath::Lerp(SocketOffset, FVector(0.f, CurrentCameraOffsetY, CurrentCameraOffsetZ), ArmData->WalkingExtensionSpeed);
}

void UMySpringArmComponent::SetRotationSpeed()
{
	if (bShouldRotateFast && RotationSpeed == ArmData->AimingRotationSpeed)
		return;
	
	if (bShouldRotateFast && UGameplayStatics::GetGlobalTimeDilation(GetWorld()) == Player->GetSlowMotionTimeDilation())
		RotationSpeed = ArmData->AimingRotationSpeed;
	else
		RotationSpeed = ArmData->StandardRotationSpeed;
}

void UMySpringArmComponent::StateSwitch(EPlayerState State)
{
	switch(State)
	{
	case Eps_Walking:
		CameraLagSpeed = 20.f;
		CameraLagMaxDistance = 3.f;
		break;
	case Eps_Sprinting:
		break;
	case Eps_Idle:
		break;
	case Eps_Aiming:
		bShouldRotateFast = true;
		CameraRotationLagSpeed = 1000.f;
		break;
	case Eps_LeaveAiming:
		bShouldRotateFast = false;
		CameraRotationLagSpeed = 50.f;
		break;
	case Eps_Climbing:
		CameraLagSpeed = 1.f;
		CameraLagMaxDistance = 10.f;
		break;
	default:
		break;
	}
	CurrentState = State;
}

void UMySpringArmComponent::UpdateWallIsInFront(bool WallIsInFront)
{
	bWallIsInFront = WallIsInFront;
}

void UMySpringArmComponent::TickStateSwitch()
{
	switch(CurrentState)
	{
	case Eps_Walking:
		TargetArmLength = FMath::Lerp(TargetArmLength, ArmData->WalkingSpringArmLength, ArmData->WalkingExtensionSpeed);
		break;
	case Eps_Sprinting:
		TargetArmLength = FMath::Lerp(TargetArmLength, ArmData->SprintingSpringArmLength, ArmData->SprintExtensionSpeed);
		break;
	case Eps_Idle:
		TargetArmLength = FMath::Lerp(TargetArmLength, ArmData->IdleSpringArmLength, ArmData->IdleExtensionSpeed);
		break;
	case Eps_Aiming:
		TargetArmLength = FMath::Lerp(TargetArmLength, ArmData->AimingSpringArmLength, ArmData->AimingExtensionSpeed);
		SocketOffset = FMath::Lerp(SocketOffset, ArmData->AimingCameraOffset, ArmData->AimingOffsetSpeed);
		break;
	case Eps_LeaveAiming:
		TargetArmLength = FMath::Lerp(TargetArmLength, ArmData->HookshotSpringArmLength, ArmData->HookshotExtensionSpeed);
		break;
	case Eps_Climbing:
		TargetOffset = FMath::Lerp(TargetOffset, Player->GetActorForwardVector() * ArmData->ClimbingSpringArmTargetOffset, ArmData->ClimbingOffsetSpeed);
		TargetArmLength = FMath::Lerp(TargetArmLength, ArmData->ClimbingSpringArmLength, ArmData->ClimbingExtensionSpeed);
		break;
	default:
		break;
	}
}

void UMySpringArmComponent::CheckWallBehindPlayer()
{
	const auto World = GetWorld();
	
	const bool bHasNoWallForDuration = TimeSinceWallBehindPlayer > ArmData->ResetTimeWallBehindPlayer;
	if (!bHasNoWallForDuration)
		TimeSinceWallBehindPlayer += World->DeltaTimeSeconds;
	
	if (CurrentState != Eps_Climbing && bWallIsInFront && bHasNoWallForDuration)
		return;
	
	// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, EDrawDebugTrace::ForOneFrame);
	if (!bHasNoWallForDuration)
		TargetArmLength = FMath::Lerp(TargetArmLength, ArmData->ArmLengthWallBehindPlayer, ArmData->SprintExtensionSpeed);

	const auto ActorLocation = Player->GetLocation();
	const auto ForwardVector = Player->GetActorForwardVector();
	const auto TraceStart = ActorLocation - ForwardVector * 20;
	const auto TraceEnd = ActorLocation - ForwardVector * ArmData->TraceLengthWallBehindPlayer;
	
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);
	
	const auto Trace = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ArmData->BlockAllCollision, Params, FCollisionResponseParams());
	if (Trace)
		TimeSinceWallBehindPlayer = 0;
}

void UMySpringArmComponent::CameraLerp(float& Value, const float Speed, const float Clamp) const
{
	Value += Speed * GetWorld()->DeltaTimeSeconds;
	Value = FMath::Clamp(Value, -Clamp, Clamp);
}

void UMySpringArmComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (IsValid(Player))
	{
		Player->OnStateChanged.AddUniqueDynamic(this, &UMySpringArmComponent::StateSwitch);
		Player->OnCanJumpBackChanged.AddUniqueDynamic(this, &UMySpringArmComponent::UpdateWallIsInFront);
	}
	
	if (!ArmData)
		UE_LOG(LogTemp, Error, TEXT("MySpringArmComponent.cpp - Data asset missing!"));

	RotationSpeed = ArmData->StandardRotationSpeed;
}

void UMySpringArmComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(Player))
		return;
	
	TickStateSwitch();
	CheckWallBehindPlayer();
	CameraMovementOutput();
	CameraOffsetByMovement();
	SetCameraOffset();
	SetRotationSpeed();
}