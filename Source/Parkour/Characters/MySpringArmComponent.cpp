// Fill out your copyright notice in the Description page of Project Settings.


#include "../Characters/MySpringArmComponent.h"

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
	CurrentCameraSpeed = StandardCameraSpeed;
}

void UMySpringArmComponent::CameraMovementOutput()
{
	if (!IsValid(Player))
		return;
	
	if (!IsValid(Player->Controller))
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter Line 125, no controller"));
		return;
	}
	
	const FVector2D CameraMove = Player->GetCameraInput();
	
	if (CameraMove == FVector2D(0, 0))
		return;
	
	// Add rotation
	Player->AddControllerYawInput(CameraMove.X * CurrentCameraSpeed * GetWorld()->DeltaTimeSeconds);
	Player->AddControllerPitchInput(-CameraMove.Y * CurrentCameraSpeed * GetWorld()->DeltaTimeSeconds);
}

void UMySpringArmComponent::CalculateCameraOffset(FVector2D CameraMove)
{
	const FVector CharacterInput = Player->GetCharacterInput();
	const auto CharacterMovement = Player->GetCharacterMovement();

	// Set camera offset in relation to character based on character movement
	// Check if character is moving 
	if (CharacterMovement->Velocity.Length() > 0.1f)
	{
		constexpr float LeftThreshold = 1.8f;
		constexpr float RightThreshold = 0.8f;

		// --------- Character movement doesn't seem to work correctly?!?!?! -------------
		
		// Sideways camera movement. Check if character's movement is towards left or right side of camera. 
		if ((CharacterInput.Rotation().Vector() - GetRightVector()).Length() > LeftThreshold)
			CameraLerp(CurrentCameraOffsetY, -CameraYDirectionSpeed, CameraClamp.Y);
		
		else if ((CharacterInput.Rotation().Vector() - GetRightVector()).Length() < RightThreshold)
			CameraLerp(CurrentCameraOffsetY, CameraYDirectionSpeed, CameraClamp.Y);
	
		// Upwards camera movement (while climbing!)
		if (CurrentState == Eps_Climbing
		&& (CharacterInput.Rotation().Vector() - GetUpVector()).Length() > LeftThreshold)
			CameraLerp(CurrentCameraOffsetZ, -CameraYDirectionSpeed, CameraClamp.Z);
		
		else if (CurrentState == Eps_Climbing
		&& (CharacterInput.Rotation().Vector() - GetUpVector()).Length() < RightThreshold)
			CameraLerp(CurrentCameraOffsetZ, CameraYDirectionSpeed, CameraClamp.Z);
	}
	
	// Camera offset by mouse input 
	constexpr float InputSensitivityThreshold = 0.005f;
	
	float CameraSpeed = CameraYDirectionSpeed / (CharacterInput.Length() + 1);
	if (CurrentState == Eps_Climbing)
	{
		CameraSpeed = CameraYDirectionSpeed * 2;
		if (CameraMove.Y < -InputSensitivityThreshold)
			CameraLerp(CurrentCameraOffsetZ, -CameraSpeed, CameraClamp.Z);
		else if (CameraMove.Y > InputSensitivityThreshold)
			CameraLerp(CurrentCameraOffsetZ, CameraSpeed, CameraClamp.Z);
	}
	
	if (CameraMove.X < -InputSensitivityThreshold)
		CameraLerp(CurrentCameraOffsetY, -CameraSpeed, CameraClamp.Y);
	else if (CameraMove.X > InputSensitivityThreshold)
		CameraLerp(CurrentCameraOffsetY, CameraSpeed, CameraClamp.Y);
}

void UMySpringArmComponent::SetCameraOffset()
{
	if (CurrentState != Eps_Climbing)
	{
		CurrentCameraOffsetZ = FMath::Lerp(CurrentCameraOffsetZ, 0.f, NormalCameraSwitchSpeed);
		TargetOffset = FMath::Lerp(TargetOffset, FVector(0, 0, 0), SpringArmSwitchSpeed);
	}

	if (CurrentState != Eps_Aiming)
		SocketOffset = FMath::Lerp(SocketOffset, FVector(0.f, CurrentCameraOffsetY, CurrentCameraOffsetZ), NormalCameraSwitchSpeed);
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
		CurrentCameraSpeed = AimCameraSpeed;
		CameraRotationLagSpeed = 1000.f;
		break;
	case Eps_LeaveAiming:
		CurrentCameraSpeed = StandardCameraSpeed;
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

void UMySpringArmComponent::UpdateCanJumpBack(bool CanJumpBack)
{
	bCanJumpBack = CanJumpBack;
}

void UMySpringArmComponent::TickStateSwitch()
{
	switch(CurrentState)
	{
	case Eps_Walking:
		TargetArmLength = FMath::Lerp(TargetArmLength, StandardSpringArmLength, NormalCameraSwitchSpeed);
		break;
	case Eps_Sprinting:
		TargetArmLength = FMath::Lerp(TargetArmLength, SprintingSpringArmLength, SpringArmSwitchSpeed);
		break;
	case Eps_Idle:
		TargetArmLength = FMath::Lerp(TargetArmLength, 10000.f, 0.0001f);
		break;
	case Eps_Aiming:
		TargetArmLength = FMath::Lerp(TargetArmLength, 100.f, 0.3f);
		SocketOffset = FMath::Lerp(SocketOffset, AimingCameraOffset, AimingCameraTransitionAlpha);
		break;
	case Eps_LeaveAiming:
		TargetArmLength = FMath::Lerp(TargetArmLength, StopAimingSpringArmLength, NormalCameraSwitchSpeed);
		break;
	case Eps_Climbing:
		TargetOffset = FMath::Lerp(TargetOffset, Player->GetActorForwardVector() * ClimbingSpringArmTargetOffset, SpringArmSwitchSpeed);
		TargetArmLength = FMath::Lerp(TargetArmLength, StandardSpringArmLength, NormalCameraSwitchSpeed);
		break;
	default:
		break;
	}
}

void UMySpringArmComponent::CheckWallBehindPlayer()
{
	if (!IsValid(Player))
		return;
	
	const auto World = GetWorld();
	
	const bool bHasNoWallForDuration = TimeSinceWallBehindPlayer > ResetTimeWallBehindPlayer;
	if (!bHasNoWallForDuration)
		TimeSinceWallBehindPlayer += World->DeltaTimeSeconds;
	
	if (CurrentState != Eps_Climbing && bCanJumpBack /*&& !Player->bHasReachedWallWhileSprinting*/ && bHasNoWallForDuration)
		return;
	
	// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, EDrawDebugTrace::ForOneFrame);
	if (!bHasNoWallForDuration)
		TargetArmLength = FMath::Lerp(TargetArmLength, ArmLengthWallBehindPlayer, SpringArmSwitchSpeed);

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
		Player->OnCanJumpBackChanged.AddUniqueDynamic(this, &UMySpringArmComponent::UpdateCanJumpBack);
	}
}

void UMySpringArmComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickStateSwitch();
	CheckWallBehindPlayer();
	CameraMovementOutput();
	SetCameraOffset();
}