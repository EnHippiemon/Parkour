// Fill out your copyright notice in the Description page of Project Settings.


#include "../Characters/MySpringArmComponent.h"

#include "MyCameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UMySpringArmComponent::UMySpringArmComponent()
{
	UMySpringArmComponent::SetAutoActivate(true);

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	bUsePawnControlRotation = true;
	bEnableCameraRotationLag = true;
	CameraRotationLagSpeed = 50.f;
	CameraLagSpeed = 20.f;
	CameraLagMaxDistance = 3.f;
	RotationSpeed = StandardRotationSpeed;
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
	
	float CameraSpeed = CameraYDirectionSpeed / (CharacterForwardVector.Length() + 1);
	if (CurrentState == Eps_Climbing)
	{
		CameraSpeed = CameraYDirectionSpeed * 2;
		if (CameraMove.Y < -InputSensitivityThreshold)
			CameraLerp(CurrentCameraOffsetZ, -CameraSpeed, OffsetClamp.Z);
		else if (CameraMove.Y > InputSensitivityThreshold)
			CameraLerp(CurrentCameraOffsetZ, CameraSpeed, OffsetClamp.Z);
	}
	
	if (CameraMove.X < -InputSensitivityThreshold)
		CameraLerp(CurrentCameraOffsetY, -CameraSpeed, OffsetClamp.Y);
	else if (CameraMove.X > InputSensitivityThreshold)
		CameraLerp(CurrentCameraOffsetY, CameraSpeed, OffsetClamp.Y);
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
		CameraLerp(CurrentCameraOffsetY, -CameraYDirectionSpeed, OffsetClamp.Y);
	else if ((CharacterMovement.Rotation().Vector() - CameraRightVector).Length() < RightThreshold)
		CameraLerp(CurrentCameraOffsetY, CameraYDirectionSpeed, OffsetClamp.Y);

	if (CurrentState != Eps_Climbing)
		return;
	
	constexpr float UpThreshold = 0.5f;
	constexpr float DownThreshold = -0.5f;

	// Upwards camera movement (while climbing!)
	if (Player->GetMovementInput().Z > UpThreshold)
		CameraLerp(CurrentCameraOffsetZ, CameraYDirectionSpeed, OffsetClamp.Z);
	else if (Player->GetMovementInput().Z < DownThreshold)
		CameraLerp(CurrentCameraOffsetZ, -CameraYDirectionSpeed, OffsetClamp.Z);
}

void UMySpringArmComponent::SetCameraOffset()
{
	if (CurrentState != Eps_Climbing)
	{
		CurrentCameraOffsetZ = FMath::Lerp(CurrentCameraOffsetZ, 0.f, WalkingExtensionSpeed);
		TargetOffset = FMath::Lerp(TargetOffset, FVector(0, 0, 0), SprintExtensionSpeed);
	}

	if (CurrentState != Eps_Aiming)
		SocketOffset = FMath::Lerp(SocketOffset, FVector(0.f, CurrentCameraOffsetY, CurrentCameraOffsetZ), WalkingExtensionSpeed);
}

void UMySpringArmComponent::SetRotationSpeed()
{
	if (bShouldRotateFast && RotationSpeed == AimingRotationSpeed)
		return;
	
	if (bShouldRotateFast && UGameplayStatics::GetGlobalTimeDilation(GetWorld()) == Player->GetSlowMotionTimeDilation())
		RotationSpeed = AimingRotationSpeed;
	else
		RotationSpeed = StandardRotationSpeed;
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
		TargetArmLength = FMath::Lerp(TargetArmLength, WalkingSpringArmLength, WalkingExtensionSpeed);
		break;
	case Eps_Sprinting:
		TargetArmLength = FMath::Lerp(TargetArmLength, SprintingSpringArmLength, SprintExtensionSpeed);
		break;
	case Eps_Idle:
		TargetArmLength = FMath::Lerp(TargetArmLength, IdleSpringArmLength, IdleExtensionSpeed);
		break;
	case Eps_Aiming:
		TargetArmLength = FMath::Lerp(TargetArmLength, AimingSpringArmLength, AimingExtensionSpeed);
		SocketOffset = FMath::Lerp(SocketOffset, AimingCameraOffset, AimingOffsetSpeed);
		break;
	case Eps_LeaveAiming:
		TargetArmLength = FMath::Lerp(TargetArmLength, HookshotSpringArmLength, HookshotExtensionSpeed);
		break;
	case Eps_Climbing:
		TargetOffset = FMath::Lerp(TargetOffset, Player->GetActorForwardVector() * ClimbingSpringArmTargetOffset, ClimbingOffsetSpeed);
		TargetArmLength = FMath::Lerp(TargetArmLength, ClimbingSpringArmLength, ClimbingExtensionSpeed);
		break;
	default:
		break;
	}
}

void UMySpringArmComponent::CheckWallBehindPlayer()
{
	const auto World = GetWorld();
	
	const bool bHasNoWallForDuration = TimeSinceWallBehindPlayer > ResetTimeWallBehindPlayer;
	if (!bHasNoWallForDuration)
		TimeSinceWallBehindPlayer += World->DeltaTimeSeconds;
	
	if (CurrentState != Eps_Climbing && bWallIsInFront /*&& !Player->bHasReachedWallWhileSprinting*/ && bHasNoWallForDuration)
		return;
	
	// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, EDrawDebugTrace::ForOneFrame);
	if (!bHasNoWallForDuration)
		TargetArmLength = FMath::Lerp(TargetArmLength, ArmLengthWallBehindPlayer, SprintExtensionSpeed);

	const auto ActorLocation = Player->GetLocation();
	const auto ForwardVector = Player->GetActorForwardVector();
	const auto TraceStart = ActorLocation - ForwardVector * 20;
	const auto TraceEnd = ActorLocation - ForwardVector * TraceLengthWallBehindPlayer;
	
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);
	
	const auto Trace = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, BlockAllCollision, Params, FCollisionResponseParams());
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