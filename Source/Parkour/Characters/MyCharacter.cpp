#include "MyCharacter.h"

#include "MySpringArmComponent.h"
#include "MyMovementModeComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyCameraComponent.h"
#include "MyCharacterMovementDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

AMyCharacter::AMyCharacter()
{
	MyMovementModeComponent = CreateDefaultSubobject<UMyMovementModeComponent>("MovementModeComponent");
	// MyCharacterMovementDataAsset->CreateDefaultSubobject<UMyCharacterMovementDataAsset>("MovementDataAsset");
}

void AMyCharacter::PlayerStateSwitch()
{
	// Checks state once when it has changed 
	if (CurrentState != PreviousState)
	{
		switch (CurrentState)
		{
		case Eps_Walking:
			GetCharacterMovement()->SetWalkableFloorAngle(50.f);
			GetCharacterMovement()->bOrientRotationToMovement = true;
			if (!bIsExhausted)
				MyMovementModeComponent->SetCurrentMovementMode(Ecmm_Walking);
			break;
		case Eps_Sprinting:
			GetCharacterMovement()->SetWalkableFloorAngle(90.f);
			MyMovementModeComponent->SetCurrentMovementMode(Ecmm_Sprinting);
			break;
		case Eps_Idle:
			MyMovementModeComponent->SetCurrentMovementMode(Ecmm_Idle);
			break;
		case Eps_Aiming:
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SlowMotionDilation);
			GetCharacterMovement()->bOrientRotationToMovement = false;
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
			GetCharacterMovement()->RotationRate = FRotator(0.f, AimRotationRate, 0.f);
			MyMovementModeComponent->SetCurrentMovementMode(Ecmm_Aiming);
			break;
		case Eps_LeaveAiming:
			GetCharacterMovement()->RotationRate = FRotator(0.f, StandardRotationRate, 0.f);
			GetCharacterMovement()->bOrientRotationToMovement = true;
			GetCharacterMovement()->bUseControllerDesiredRotation = false;
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
			if (bIsUsingHookshot)
				MyMovementModeComponent->SetCurrentMovementMode(Ecmm_LeavingAim);
			break;
		case Eps_Climbing:
			GetCharacterMovement()->MovementMode = MOVE_Flying;
			GetCharacterMovement()->bOrientRotationToMovement = false;
			MyMovementModeComponent->SetCurrentMovementMode(Ecmm_Climbing);
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("No active Player State. Now Walking"))
		}
		OnStateChanged.Broadcast(CurrentState);
	}
	// Move this up one line 
	PreviousState = CurrentState;

	// Checks state every tick  
	switch (CurrentState)
	{
	case Eps_Walking:
		CheckFloorAngle();
		CheckIdleness();
		TargetMovementSpeed = MaxWalkSpeed * MovementSpeedPercent * MovementEnergy;
		break;
	case Eps_Sprinting:
		CheckFloorAngle();
		TargetMovementSpeed = MaxSprintSpeed * MovementSpeedPercent * MovementEnergy;
		if (GetCharacterMovement()->Velocity.Length() < 0.1f)
			HandleSprintStop();
		break;
	case Eps_Idle:
		CheckIdleness();
		AddControllerYawInput(GetWorld()->DeltaTimeSeconds * 2);
		if (SpringArm->GetTargetRotation().Vector().Z > -0.25f)
			AddControllerPitchInput(GetWorld()->DeltaTimeSeconds / 2);
		else if (SpringArm->GetTargetRotation().Vector().Z < -0.3f)
			AddControllerPitchInput(-GetWorld()->DeltaTimeSeconds / 2);
		
		if (TimeSinceMoved < 2.f)
			CurrentState = SavedState;
		break;		
	case Eps_Aiming:
		MovementEnergy -= GetWorld()->DeltaTimeSeconds * AimEnergyDepletionSpeed;
		break;
	case Eps_LeaveAiming:
		if (bIsUsingHookshot && (GetActorLocation() - TargetLocation).Length() < 10.f)
			bIsUsingHookshot = false;

		if (!bIsUsingHookshot && SavedState == Eps_Sprinting)
			CurrentState = Eps_Walking;
		else if (!bIsUsingHookshot)
			CurrentState = SavedState;
		break;
	case Eps_Climbing:
		// Makes sure that the player is pushed to the wall and doesn't fall off
		if (CantClimbTimer >= ClimbJumpingTime)
			GetCharacterMovement()->AddImpulse(GetActorForwardVector() * 1000.f);
		break;		
	default:
		UE_LOG(LogTemp, Error, TEXT("No active Player State. Now Walking"))
		CurrentState = Eps_Walking;
		break;
	}
			
	if (bIsUsingHookshot || bHasReachedWallWhileSprinting)
		SetPlayerVelocity(FVector(0, 0, 0));
}

void AMyCharacter::MovementOutput()
{
	if (Controller == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter Line 99, no controller"));
		return;
	}
	
	if (CurrentState == Eps_Climbing)
		CharacterMovement = FVector(0.f, GetMovementSideways(), GetMovementForward());
	else
		CharacterMovement = FVector(GetMovementForward(), GetMovementSideways(), 0.f);
	
	CharacterMovement.Normalize();

	if (CurrentState == Eps_Climbing)
	{
		// Climb in relation to character's rotation 
		CharacterMovement = GetActorForwardVector().Rotation().RotateVector(CharacterMovement);
	}
	else 
	{
		// Walk toward camera's forward vector 
		const FRotator CameraRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, CameraRotation.Yaw, 0.f);
		CharacterMovement = YawRotation.RotateVector(CharacterMovement);
	}

	if (CharacterMovement.Length() == 0)
	{
		constexpr int SpeedToStop = 20;
		if (GetCharacterMovement()->MaxWalkSpeed < SpeedToStop || GetIsMidAir())
			bShouldStopMovementOverTime = false;
		
		SetMovementSpeed(0);
		StopMovementOverTime();
	}
	else
	{
		if (GetCharacterMovement()->MaxWalkSpeed < ThresholdToStopOverTime)
			bShouldStopMovementOverTime = false;
		
		SetMovementSpeed(TargetMovementSpeed);
		AddMovementInput(CharacterMovement);
	}
	// UE_LOG(LogTemp, Log, TEXT("Movement: %f"), GetCharacterMovement()->MaxWalkSpeed);
}

void AMyCharacter::SetPlayerVelocity(const FVector& Value) const
{
	GetCharacterMovement()->Velocity = Value;
}

// void AMyCharacter::SetCurrentMovementMode(ECurrentMovementMode Movement)
// {
// 	if (MovementMode == Movement)
// 		return;
// 	
// 	MovementMode = Movement;
//
// 	switch (MovementMode)
// 	{
// 	case Ecmm_Idle:
// 		CurrentMovementTexture = IdleTexture;
// 		break;
// 	case Ecmm_Walking:
// 		CurrentMovementTexture = WalkingTexture;
// 		break;
// 	case Ecmm_Sprinting:
// 		CurrentMovementTexture = RunningTexture;
// 		break;
// 	case Ecmm_Climbing:
// 		CurrentMovementTexture = ClimbingTexture;
// 		break;
// 	case Ecmm_LedgeClimbing:
// 		CurrentMovementTexture = LedgeClimbingTexture;
// 		break;
// 	case Ecmm_Jumping:
// 		CurrentMovementTexture = JumpTexture;
// 		break;
// 	case Ecmm_ClimbJumping:
// 		CurrentMovementTexture = ClimbJumpTexture;
// 		break;
// 	case Ecmm_RunningUpWall:
// 		CurrentMovementTexture = RunUpWallTexture;
// 		break;
// 	case Ecmm_WallJumping:
// 		CurrentMovementTexture = WallJumpTexture;
// 		break;
// 	case Ecmm_Aiming:
// 		CurrentMovementTexture = AimingTexture;
// 		break;
// 	case Ecmm_LeavingAim:
// 		CurrentMovementTexture = LeaveAimingTexture;
// 		break;
// 	case Ecmm_Exhausted:
// 		CurrentMovementTexture = ExhaustedTexture;
// 		break;
// 	default:
// 		CurrentMovementTexture = WalkingTexture;
// 	}
// 	UE_LOG(LogTemp, Log, TEXT("Movement mode: %d"), MovementMode.GetValue())
// 	OnNewMovement.Broadcast();
// }

void AMyCharacter::CheckFloorAngle()
{
	// Cache getters to increase performance 
	const auto World = GetWorld();
	const auto RightVector = GetActorRightVector();
	constexpr int TraceLengthForward = 150;
	constexpr int TraceLengthDown = 300;

	// Trace vectors 
	const FVector StartFloorTrace = GetActorLocation();
	const FVector EndFloorTrace = StartFloorTrace + GetActorForwardVector() * TraceLengthForward - GetActorUpVector() * TraceLengthDown;

	constexpr int TraceSeparation = 20;
	int WidestPossibleTrace = 130;
	// Increase the width to prevent zig zag cheating. 
	if (!bCanGainEnergy)
		WidestPossibleTrace = 300;
	TArray<float> TraceDistances;

	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);
	
	int CurrentTrace = 0;
	for (int i = -WidestPossibleTrace; i < WidestPossibleTrace; ++i)
	{
		// Check both left and right sides at the same time 
		if (i % TraceSeparation == 1 || i % TraceSeparation == -1)
		{
			FHitResult HitResult;
			const FVector TraceEnds = FVector(EndFloorTrace + RightVector * i);
			const auto Trace = World->LineTraceSingleByChannel(HitResult, StartFloorTrace,
								   TraceEnds, BlockAllCollision, Parameters, FCollisionResponseParams());

			// If the line trace is null, its distance is automatically 0. Make it max float so the others are shorter. 
			if (!Trace)
				HitResult.Distance = FLT_MAX;

			TraceDistances.Add(HitResult.Distance);
			++CurrentTrace;

			// Debug line for illustration in portfolio 
			// DrawDebugLine(World, StartFloorTrace, TraceEnds, FColor::Red, false, EDrawDebugTrace::ForOneFrame, 0, 1.f);
		}
	}
	
	// Get the shortest trace. Divide by 100 to make it 0-1. 
	FloorAngle = FindSmallestFloat(TraceDistances) * 0.01f;

	// Decide if the player should slide down a wall 
	if (FloorAngle < ThresholdToJumpBack && GetCharacterMovement()->Velocity.Z < 0.f && !bIsExhausted && CurrentState != Eps_Climbing)
	{
		SetPlayerVelocity(FVector(0.f, 0.f, -250.f));
		GetCharacterMovement()->GravityScale = 0.f;
		GetCharacterMovement()->JumpZVelocity = 0;
	}
	else
	{
		GetCharacterMovement()->GravityScale = 2.f;
		GetCharacterMovement()->JumpZVelocity = 763;
	}
}

void AMyCharacter::CheckExhaustion()
{
	float MovementLoss = 0.f;
	
	if (MovementEnergy >= 1.f && bIsExhausted)
	{
		MyMovementModeComponent->SetCurrentMovementMode(Ecmm_Walking);
		bIsExhausted = false;
	}
	
	if (MovementEnergy <= 0.f && !bIsExhausted)
	{
		MyMovementModeComponent->SetCurrentMovementMode(Ecmm_Exhausted);
		bIsExhausted = true;
		CurrentState = CurrentState == Eps_Aiming ? Eps_LeaveAiming : Eps_Walking;
	}

	const auto CorrectedFloorAngle = -FloorAngle + 1.007908f;
	const auto ScaledFloorAngle = UKismetMathLibrary::NormalizeToRange(CorrectedFloorAngle, 0, 0.4f);
	bool UsingEnergy;
	
	// UE_LOG(LogTemp, Warning, TEXT("Corrected floor: %f | Scaled floor: %f | Floor angle: %f"), CorrectedFloorAngle, ScaledFloorAngle, FloorAngle);
	
	if (CurrentState == Eps_Sprinting)
	{
		// Checking if on ground
		constexpr int MinDistanceToGround = 10;
		const FVector TraceDownStart = GetActorLocation() - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() *
									   GetActorUpVector() - GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector TraceDownEnd = TraceDownStart - GetActorUpVector() * MinDistanceToGround;
		// DrawDebugLine(GetWorld(), TraceDownStart, TraceDownEnd, FColor::Red, false, EDrawDebugTrace::ForOneFrame);
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		const auto TraceDown = GetWorld()->LineTraceSingleByChannel(HitResult, TraceDownStart, TraceDownEnd, BlockAllCollision, Params, FCollisionResponseParams());
	
		if (ScaledFloorAngle > FloorAngleThreshold && !TraceDown)
		{
			bCanGainEnergy = false;
			UsingEnergy = true;
			MovementLoss = GetWorld()->DeltaTimeSeconds * ExhaustionSpeed * ScaledFloorAngle;			
		}
		else
		{
			bCanGainEnergy = true;
			UsingEnergy = false;
		}
	}
	else
		UsingEnergy = false;

	if (ScaledFloorAngle < FloorAngleThreshold)
		bCanGainEnergy = true;
	
	if (MovementEnergy < 1.f && !UsingEnergy && bCanGainEnergy)
		MovementEnergy += GetWorld()->DeltaTimeSeconds * EnergyRegainSpeed;
	
	// Decide movement speed
	if (GetIsMidAir())
		return;
	
	// Flip 0 and 1 so movement speed is increased with higher value. 
	MovementSpeedPercent = FMath::Clamp(ScaledFloorAngle * - 1 + 1, 0, 2);

	MovementEnergy -= MovementLoss;
}

void AMyCharacter::HandleJumpInput()
{
	if (bIsExhausted)
		return;
	
	// When running up wall 
	if (bHasReachedWallWhileSprinting)
	{
		// Interrupt running
		FLatentActionManager LatentActionManager = GetWorld()->GetLatentActionManager();
		LatentActionManager.RemoveActionsForObject(this);

		// Jump in backward direction 
		SetPlayerVelocity(FVector(0.f, 0.f, WallJumpUpVelocity) - GetActorForwardVector() * WallJumpBackVelocity);
		SetActorRotation(FRotator(0, GetActorRotation().Yaw + 180, 0));
		bHasReachedWallWhileSprinting = false;
		MyMovementModeComponent->SetCurrentMovementMode(Ecmm_WallJumping);
		return;
	}

	// Jump while climbing 
	if (CurrentState == Eps_Climbing && CantClimbTimer >= ClimbJumpingTime)
	{
		CantClimbTimer = 0.f;
		MovementEnergy -= 0.3f;
		GetCharacterMovement()->BrakingDecelerationFlying = 1000.f;

		// Jump backward, straight out from wall 
		if (GetCharacterMovement()->Velocity.Length() == 0)
		{
			constexpr int RotationCorrection = 180;
			bIsJumpingOutFromWall = true;
			MyMovementModeComponent->SetCurrentMovementMode(Ecmm_WallJumping);
			SetPlayerVelocity(FVector(0.f, 0.f, VelocityClimbJumpOutUp) - GetActorForwardVector() * VelocityClimbJumpOutBack);
			SetActorRotation(FRotator(0, GetActorRotation().Yaw + RotationCorrection, 0));
			return;
		}

		MyMovementModeComponent->SetCurrentMovementMode(Ecmm_ClimbJumping);
		
		// Jump in direction of movement input 
		SetPlayerVelocity(CharacterMovement * WallJumpUpVelocity);
		return;
	}

	// Wall jumping
	constexpr int MinDistanceToGround = 25;
	const FVector TraceDownStart = GetActorLocation() - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() *
								   GetActorUpVector() - GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector TraceDownEnd = TraceDownStart - GetActorUpVector() * MinDistanceToGround;
	// DrawDebugLine(GetWorld(), TraceDownStart, TraceDownEnd, FColor::Red, false, 3);
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	const auto TraceDown = GetWorld()->LineTraceSingleByChannel(HitResult, TraceDownStart, TraceDownEnd, BlockAllCollision, Params, FCollisionResponseParams());
	
	if (GetCanJumpBackwards() && !TraceDown)
	{
		MyMovementModeComponent->SetCurrentMovementMode(Ecmm_WallJumping);

		if (GetCharacterMovement()->IsMovingOnGround())
		{
			if (CurrentState == Eps_Sprinting)
			{
				constexpr int SprintCompensationForce = 100;
				GetCharacterMovement()->AddImpulse(FVector(0.f, 0.f, WallJumpUpVelocity * SprintCompensationForce) + CharacterMovement * WallJumpBackVelocity * SprintCompensationForce);
			}
			else
				GetCharacterMovement()->AddImpulse(CharacterMovement * WallJumpBackVelocity);
		}
		else
			SetPlayerVelocity(FVector(0.f, 0.f, WallJumpUpVelocity) + CharacterMovement * WallJumpBackVelocity);
		return;
	}

	if (GetCharacterMovement()->IsMovingOnGround())
	{
		MyMovementModeComponent->SetCurrentMovementMode(Ecmm_Jumping);
		MovementEnergy -= 0.3f;
		GetCharacterMovement()->AddImpulse(FVector(0, 0, RegularJumpForce));
	}
}

void AMyCharacter::Landed(const FHitResult& Hit)
{
	Super::Super::Landed(Hit);

	if (!bIsExhausted)
		MyMovementModeComponent->SetCurrentMovementMode(CurrentState == Eps_Sprinting ? Ecmm_Sprinting : Ecmm_Walking);
	
	bHasReachedWallWhileSprinting = false;
}

void AMyCharacter::HandleSprintInput()
{
	if (CurrentState != Eps_Walking || bIsExhausted || GetCharacterMovement()->Velocity.Length() < 0.1f)
		return;
	
	CurrentState = Eps_Sprinting;
}

void AMyCharacter::HandleSprintStop()
{
	if (CurrentState != Eps_Sprinting || bHasReachedWallWhileSprinting)
		return;

	CurrentState = Eps_Walking;
}

void AMyCharacter::SetMovementSpeed(const float TargetSpeed) const
{
	FMath::Clamp(TargetSpeed, 0, MaxSprintSpeed);
	const auto Alpha = TargetSpeed < GetCharacterMovement()->MaxWalkSpeed ? ReachTargetDownSpeed : ReachTargetUpSpeed;

	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(
		GetCharacterMovement()->MaxWalkSpeed,
		TargetSpeed,
		Alpha * GetWorld()->DeltaTimeSeconds
		);
}

void AMyCharacter::StopMovementOverTime()
{
	CheckShouldStopMovementOverTime();

	if (!bShouldStopMovementOverTime)
		return;
	
	const auto Movement = GetActorForwardVector() * GetCharacterMovement()->MaxWalkSpeed;
	AddMovementInput(Movement);
}

void AMyCharacter::CheckShouldStopMovementOverTime()
{
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	constexpr int DistanceToObstacle = 10;
	const FVector TraceStart = GetActorLocation();
	const FVector TraceEnd = GetActorLocation() + GetActorForwardVector() * DistanceToObstacle;	

	const auto ObstacleTrace  = UKismetSystemLibrary::CapsuleTraceSingle(
	GetWorld(),
	TraceStart,
	TraceEnd,
	GetCapsuleComponent()->GetScaledCapsuleRadius(),
	GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
	ObstacleTraceType,
	false,
	{this},
	EDrawDebugTrace::None,
	HitResult,
	true,
	FLinearColor::Red,
	FLinearColor::Green,
	1.f
	);

	if (ObstacleTrace)
	{
		bShouldStopMovementOverTime = false;
		return;
	}
	
	if (bShouldStopMovementOverTime || GetCharacterMovement()->MaxWalkSpeed < ThresholdToStopOverTime)
		return;
	
	bShouldStopMovementOverTime = true;
}

void AMyCharacter::CheckIfWallIsInFront()
{
	// If value hasn't changed, don't do anything
	if (bWallIsInFront == FloorAngle > ThresholdToJumpBack)
		return;

	bWallIsInFront = FloorAngle > ThresholdToJumpBack;
	OnCanJumpBackChanged.Broadcast(bWallIsInFront);
}

bool AMyCharacter::GetCanJumpBackwards() const
{
	return FloorAngle < ThresholdToJumpBack && (GetActorForwardVector() - CharacterMovement).Length() > 1.7f;
}

bool AMyCharacter::GetIsMidAir() const
{
	return GetCharacterMovement()->Velocity.Z == 0 ? 0 : 1;
}

void AMyCharacter::FindClimbableWall()
{
	if (bIsClimbingLedge)
		return;
	
	if (CantClimbTimer < ClimbJumpingTime)
	{
		CantClimbTimer += GetWorld()->DeltaTimeSeconds;
		FindClimbRotation();
		return;
	}

	const auto World = GetWorld();
	const auto ActorLocation = GetActorLocation();
	const auto ForwardVector = GetActorForwardVector();
	const auto RightVector = GetActorRightVector();

	constexpr int TraceLength = 80;
	const FVector StartWallAngle = ActorLocation;
	const FVector EndForwardAngle = ActorLocation + ForwardVector * TraceLength;
	const FVector EndRightAngle = ActorLocation + ForwardVector * TraceLength + RightVector * ClimbingWidth;
	const FVector EndLeftAngle = ActorLocation + ForwardVector * TraceLength - RightVector * ClimbingWidth;

	FHitResult HitResultForward;
	FHitResult HitResultRight;
	FHitResult HitResultLeft;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);

	const auto ForwardTrace = World->LineTraceSingleByChannel(HitResultForward, StartWallAngle, EndForwardAngle, ClimbingCollision, Parameters, FCollisionResponseParams());
	const auto RightTrace = World->LineTraceSingleByChannel(HitResultRight, StartWallAngle, EndRightAngle, ClimbingCollision, Parameters, FCollisionResponseParams());
	const auto LeftTrace = World->LineTraceSingleByChannel(HitResultLeft, StartWallAngle, EndLeftAngle, ClimbingCollision, Parameters, FCollisionResponseParams());

	// Check if two LineTraces find a climbing wall. 
	if (ForwardTrace && RightTrace ||
		ForwardTrace && LeftTrace ||
		RightTrace && LeftTrace)
	{
		if (CurrentState == Eps_Aiming)
		{
			SavedState = Eps_Climbing;
			CurrentState = Eps_LeaveAiming;
		}
		else
		{
			CurrentState = Eps_Climbing;
			MyMovementModeComponent->SetCurrentMovementMode(Ecmm_Climbing);
			GetCharacterMovement()->BrakingDecelerationFlying = FLT_MAX;
			FindClimbRotation();
		}
	}
	else if (CurrentState == Eps_Climbing)
		CancelAction();
}

void AMyCharacter::FindClimbRotation()
{
	FHitResult HitResultPlayerRotation;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);

	// Cache the getters, because all of them are used several times. 
	const auto World = GetWorld();
	const auto ActorLocation = GetActorLocation();
	const auto ForwardVector = GetActorForwardVector();
	const auto RightVector = GetActorRightVector();
	const auto UpVector = GetActorUpVector();
	const auto CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius() * 0.5f;
	const auto MovementSideways = GetMovementSideways();
	const auto MovementForward = GetMovementForward();

	TArray<FVector> StartTraces;
	TArray<FVector> EndTraces;

#pragma region Vectors
	const FVector StartBackWallDetection = ActorLocation + ForwardVector * CapsuleRadius;
	StartTraces.Add(StartBackWallDetection);
	const FVector EndBackWallDetection = ActorLocation + UpVector * MovementForward * 4000 + RightVector * MovementSideways * 4000 - ForwardVector * 30.f;
	EndTraces.Add(EndBackWallDetection);

	const FVector StartSideWallDetection = ActorLocation;
	StartTraces.Add(StartSideWallDetection);
	const FVector EndSideWallDetection = ActorLocation + MovementSideways * RightVector * 5000.f;
	EndTraces.Add(EndSideWallDetection);
	
	const FVector StartFrontWallDetection = ActorLocation - ForwardVector * CapsuleRadius;
	StartTraces.Add(StartFrontWallDetection);
	const FVector EndFrontWallDetection = ActorLocation + UpVector * MovementForward * PlayerToWallDistance + RightVector *
										  MovementSideways * PlayerToWallDistance + ForwardVector * 80.f;
	EndTraces.Add(EndFrontWallDetection);
	
	const FVector StartEyesightWallDetection = ActorLocation;
	StartTraces.Add(StartEyesightWallDetection);
	const FVector EndEyesightWallDetection = ActorLocation + ForwardVector * 100.f;
	EndTraces.Add(EndEyesightWallDetection);
#pragma endregion
	
	bool WallRotationTrace = false;
	for (int i = 0; i < StartTraces.Num(); ++i)
	{
		WallRotationTrace = World->LineTraceSingleByChannel(HitResultPlayerRotation, StartTraces[i], EndTraces[i], ClimbingCollision, Parameters, FCollisionResponseParams());
		if (WallRotationTrace)
			break;
	}

	if (!WallRotationTrace)
		return;
	
	if (bIsJumpingOutFromWall && CurrentClimbingWall == HitResultPlayerRotation.GetActor())
		return;

	// DrawDebugLine(World, StartTraces[i], EndTraces[i], FColor::Red, false, EDrawDebugTrace::ForOneFrame);
	bIsJumpingOutFromWall = false;
	CurrentClimbingWall = HitResultPlayerRotation.GetActor();
	SetPlayerRotation(HitResultPlayerRotation.ImpactNormal.Rotation());
}

void AMyCharacter::SetPlayerRotation(const FRotator& TargetRotation)
{
	SetActorRotation(FMath::Lerp(
		GetActorRotation(),
		FRotator(0.f, 180.f, 0.f)
		+ FRotator(-TargetRotation.Pitch, TargetRotation.Yaw, 0.f),
		0.05f));
}

// Interrupt climbing
void AMyCharacter::CancelAction()
{
	if (CurrentState != Eps_Climbing)
		return;
	
	CantClimbTimer = 0.f;
	CurrentState = Eps_Walking;
	GetCharacterMovement()->MovementMode = MOVE_Walking;
}

void AMyCharacter::LookForLedge()
{
	if (bIsClimbingLedge)
	{
		SetPlayerVelocity(FVector(0, 0, 0));
		if (FMath::IsNearlyZero((GetActorLocation() - LedgeClimbDestination).Length()))
			bIsClimbingLedge = false;
		return;
	}
	if (!GetIsMidAir())
		return; 

	auto World = GetWorld();
	const FVector LowTraceStart = GetActorLocation() + GetActorUpVector() * BottomLedgeDetectionZOffset;
	const FVector LowTraceEnd = LowTraceStart + GetActorForwardVector() * LedgeClimbDetectionOffset.X;
	const FVector HighTraceStart = GetActorLocation() + GetActorUpVector() * LedgeClimbDetectionOffset.Z;
	const FVector HighTraceEnd = HighTraceStart + GetActorForwardVector() * LedgeClimbDetectionOffset.X;
	
	FHitResult LowHitResult;
	FHitResult HighHitResult;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);
	const auto LowTrace = World->LineTraceSingleByChannel(LowHitResult, LowTraceStart, LowTraceEnd, LedgeChannel, Parameters, FCollisionResponseParams());
	const auto HighTrace = World->LineTraceSingleByChannel(HighHitResult, HighTraceStart, HighTraceEnd, BlockAllCollision, Parameters, FCollisionResponseParams());

	// DrawDebugLine(World, LowTraceStart, LowTraceEnd, FColor::Red, false, EDrawDebugTrace::ForOneFrame);
	// DrawDebugLine(World, HighTraceStart, HighTraceEnd, FColor::Blue, false, EDrawDebugTrace::ForOneFrame);
	if (LowTrace && !HighTrace)
	{
		LedgeClimbDestination = HighTraceEnd + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		bIsClimbingLedge = true;
		MyMovementModeComponent->SetCurrentMovementMode(Ecmm_LedgeClimbing);
		
		UKismetSystemLibrary::MoveComponentTo(
			RootComponent,
			LedgeClimbDestination,
			GetActorRotation(),
			true,
			true,
			LedgeClimbDuration,
			false,
			EMoveComponentAction::Move,
			LatentActionInfo
			);
	}
}

void AMyCharacter::CheckIdleness()
{
	TimeSinceMoved += GetWorld()->DeltaTimeSeconds;

	if (GetCharacterMovement()->Velocity.Length() != 0.f || GetCameraInput().Length() != 0.f)
		TimeSinceMoved = 0.f;
	
	// Time in seconds before perceived as idle 
	if (TimeSinceMoved > TimeBeforeIdle && CurrentState != Eps_Idle)
	{
		SavedState = CurrentState;
		CurrentState = Eps_Idle;
	}
}

void AMyCharacter::HandleSecondaryActionInput()
{
	if (CurrentState == Eps_Idle || GetCharacterMovement()->IsMovingOnGround() ||
		bIsExhausted || bHasReachedWallWhileSprinting)
			return;

	// Save current state for later
	if (CurrentState != Eps_LeaveAiming)
		SavedState = CurrentState;
	
	CurrentState = Eps_Aiming;
}

void AMyCharacter::HandleSecondaryActionStop()
{
	if (CurrentState != Eps_Aiming)
		return;

	CurrentState = Eps_LeaveAiming;
}

// Look for target to use hookshot on
void AMyCharacter::HandleActionInput()
{
	if (CurrentState != Eps_Aiming)
		return;
	
	FHitResult HookshotTarget;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);

	const FVector StartHookSearch = CameraComponent->GetComponentLocation();
	const FVector EndHookSearch = CameraComponent->GetComponentLocation() + CameraComponent->GetForwardVector() * HookLength;

	DrawDebugLine(GetWorld(), StartHookSearch, EndHookSearch, FColor::Purple, false, 0.1f, 0.f, 3.f);

	const auto HookTrace = GetWorld()->LineTraceSingleByChannel(HookshotTarget, StartHookSearch, EndHookSearch, HookCollision, Parameters, FCollisionResponseParams());
	if (HookTrace)
	{
		const FVector TargetOffset = CameraComponent->GetForwardVector() * 50.f + FVector(0, 0, 100.f);
		TargetLocation = HookshotTarget.Location - TargetOffset;
		bIsUsingHookshot = true;
		CurrentState = Eps_LeaveAiming;
		
		UKismetSystemLibrary::MoveComponentTo(
			RootComponent,
			TargetLocation,
			FRotator(0, CameraComponent->GetForwardVector().Rotation().Yaw, 0),
			false,
			false,
			HookshotTarget.Distance/HookshotSpeed,
			false,
			EMoveComponentAction::Move,
			LatentActionInfo
			);
	}
}

float AMyCharacter::FindSmallestFloat(TArray<float> Array)
{
	float SmallestValue = FLT_MAX;
	for(int i = 0; i < Array.Num(); i++)
	{
		if (Array[i] < SmallestValue)
			SmallestValue = Array[i];
	}
	
	return SmallestValue;
}

void AMyCharacter::RunUpToWall()
{
	if (CurrentState != Eps_Sprinting || bHasReachedWallWhileSprinting)
		return;
	if (GetVelocity().Z < 0)
	{
		bIsNearingWall = false;
		return;
	}

	const auto World = GetWorld();
	
	FHitResult DistancedLookForWallHitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	const FVector TraceStart = GetActorLocation();
	const FVector DistancedLookForWallEnd = GetActorLocation() + GetActorForwardVector() * DistanceBeforeAbleToRunUpWall;	
	const auto DistancedLookForWall = GetWorld()->LineTraceSingleByChannel(DistancedLookForWallHitResult, TraceStart, DistancedLookForWallEnd, BlockAllCollision, Params, FCollisionResponseParams());

	if (FoundWall != DistancedLookForWallHitResult.GetActor())
	{
		bIsNearingWall = false;
		FoundWall = DistancedLookForWallHitResult.GetActor();
	}
	
	// DrawDebugLine(GetWorld(), TraceStart, DistancedLookForWallEnd, FColor::Red, false, EDrawDebugTrace::ForOneFrame, 0, 1);

	if (!DistancedLookForWall)
		bIsNearingWall = false;
	
	FHitResult DistancedHitResult;
	const FVector DistancedTraceEnd = DistancedLookForWallEnd - GetActorForwardVector() * 50;
	const auto DistancedWallTrace = World->LineTraceSingleByChannel(DistancedHitResult, TraceStart, DistancedTraceEnd, BlockAllCollision, Params, FCollisionResponseParams());

	if (DistancedLookForWall && !DistancedWallTrace)
		bIsNearingWall = true;
	
	if (!bIsNearingWall)
		return;
	
	FHitResult ShortWallSearchHitResult;
	auto constexpr DistanceToWall = 40;
	const FVector TraceEnd = GetActorLocation() + GetActorForwardVector() * DistanceToWall;
	
	const auto LookForWall = GetWorld()->LineTraceSingleByChannel(ShortWallSearchHitResult, TraceStart, TraceEnd, BlockAllCollision, Params, FCollisionResponseParams());
	if (LookForWall && ShortWallSearchHitResult.GetActor() == FoundWall)
	{
		FHitResult CapsuleHitResult;
		auto constexpr RunDistance = 200;
		RunningUpWallEndLocation = GetActorLocation() + GetActorUpVector() * RunDistance;
		
		auto CapsuleTrace = UKismetSystemLibrary::CapsuleTraceSingle(
			GetWorld(),
			TraceStart,
			RunningUpWallEndLocation,
			GetCapsuleComponent()->GetScaledCapsuleRadius(),
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
			ObstacleTraceType,
			false,
			{this},
			EDrawDebugTrace::None,
			CapsuleHitResult,
			true,
			FLinearColor::Red,
			FLinearColor::Green,
			1.f
			);
		
		if (!CapsuleTrace)
		{
			bHasReachedWallWhileSprinting = true;
			bIsNearingWall = false;
			MyMovementModeComponent->SetCurrentMovementMode(Ecmm_RunningUpWall);
			UKismetSystemLibrary::MoveComponentTo(
				RootComponent,
				RunningUpWallEndLocation,
				GetActorRotation(),
				true,
				false,
				RunningUpWallTimeInSeconds,
				false,
				EMoveComponentAction::Move,
				LatentActionInfo
				);
		}
	}
}

void AMyCharacter::RunningUpWall()
{
	if (!bHasReachedWallWhileSprinting)
		return;
	
	SetPlayerVelocity(FVector(0, 0, 0));
	if (UKismetMathLibrary::Vector_Distance(GetActorLocation(), RunningUpWallEndLocation) < 10.f)
		bHasReachedWallWhileSprinting = false;
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	MyMovementModeComponent->SetCurrentMovementMode(Ecmm_Walking);

	if (!IsValid(MyCharacterMovementDataAsset))
		UE_LOG(LogTemp, Error, TEXT("MyCharacter.cpp - No Data Asset selected!"))
}

void AMyCharacter::Tick(float const DeltaTime)
{
	Super::Tick(DeltaTime);

	MovementOutput();
	CheckFloorAngle();
	CheckIfWallIsInFront();
	
	/* Climbing */
	CheckExhaustion();
	FindClimbableWall();
	LookForLedge();
	
	/* Running */
	RunUpToWall();
	RunningUpWall();

	// Keep last 
	PlayerStateSwitch();
}