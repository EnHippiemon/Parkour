#include "MyCharacter.h"

#include "MySpringArmComponent.h"
#include "MyMovementModeComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyClimbComponent.h"
#include "MyHookshotComponent.h"
#include "DataAssets/GroundMovementDataAsset.h"
#include "DataAssets/EnergyDataAsset.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../FStaticFunctions.h"


AMyCharacter::AMyCharacter()
{
	MyAnimationComponent = CreateDefaultSubobject<UMyMovementModeComponent>("MovementModeComponent");
	ClimbComponent = CreateDefaultSubobject<UMyClimbComponent>("ClimbComponent");
	HookshotComponent = CreateDefaultSubobject<UMyHookshotComponent>("HookshotComponent");
	
	// Hard setting the data assets 
	static ConstructorHelpers::FObjectFinder<UGroundMovementDataAsset> GroundMovement(TEXT("/Game/Player/DataAssets/GroundMovementDataAsset"));
	if (GroundMovement.Object)
		GroundMovementData = GroundMovement.Object;
	static ConstructorHelpers::FObjectFinder<UEnergyDataAsset> Energy(TEXT("/Game/Player/DataAssets/EnergyDataAsset"));
	if (Energy.Object)
		EnergyData = Energy.Object;
}

bool AMyCharacter::GetWallIsInFront()
{
	return FloorAngle > GroundMovementData->ThresholdToJumpBack;
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
			break;
		case Eps_Sprinting:
			GetCharacterMovement()->SetWalkableFloorAngle(90.f);
			break;
		case Eps_Idle:
			MyAnimationComponent->SetCurrentAnimation(Ecmm_Idle);
			break;
		case Eps_Aiming:
			break;
		case Eps_LeaveAiming:
			break;
		case Eps_Climbing:
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("No active Player State. Now Walking"))
		}

		OnStateChanged.Broadcast(CurrentState);
		PreviousState = CurrentState;
	}
	
	// Checks state every tick  
	switch (CurrentState)
	{
	case Eps_Walking:
		CheckIdleness();
		TargetMovementSpeed = GroundMovementData->MaxWalkSpeed * MovementSpeedPercent * MovementEnergy;
		if (!bIsExhausted && !GetIsMidAir() && !ClimbComponent->GetIsClimbingLedge())
			MyAnimationComponent->SetCurrentAnimation(Ecmm_Walking);
		break;
	case Eps_Sprinting:
		TargetMovementSpeed = GroundMovementData->MaxSprintSpeed * MovementSpeedPercent * MovementEnergy;
		if (GetCharacterMovement()->Velocity.Length() < 0.1f)
			HandleSprintStop();
		if (!GetIsMidAir() && !bHasReachedWallWhileSprinting && !ClimbComponent->GetIsClimbingLedge())
			MyAnimationComponent->SetCurrentAnimation(Ecmm_Sprinting);
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
		MovementEnergy -= GetWorld()->DeltaTimeSeconds * EnergyData->AimEnergyDepletionSpeed;
		if (!GetIsMidAir())
			HandleSecondaryActionStop();
		break;
	case Eps_LeaveAiming:
		CurrentState = HookshotComponent->LeaveAiming(this);
		break;
	case Eps_Climbing:
		break;		
	default:
		UE_LOG(LogTemp, Error, TEXT("No active Player State. Now Walking"))
		CurrentState = Eps_Walking;
		break;
	}
			
	if (HookshotComponent->GetIsUsingHookshot() || bHasReachedWallWhileSprinting || ClimbComponent->GetIsClimbingLedge())
		SetPlayerVelocity(FVector(0, 0, 0));
}

void AMyCharacter::MovementOutput()
{
	if (!IsValid(Controller))
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter.cpp - no controller"));
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
		if (GetCharacterMovement()->MaxWalkSpeed < GroundMovementData->ThresholdToStopOverTime)
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

			DrawDebugLine(World, StartFloorTrace, TraceEnds, FColor::Red, false, EDrawDebugTrace::ForOneFrame);
		}
	}
	
	// Get the shortest trace. Divide by 100 to make it 0-1. 
	FloorAngle = FStaticFunctions::FindSmallestFloat(TraceDistances) * 0.01f;
}

// Decide if the player should slide down a wall 
void AMyCharacter::DecideIfShouldSlide()
{
	const bool SlideRequirements = GetCanJumpBackwards() && GetCharacterMovement()->Velocity.Z < 0.f && !ClimbComponent->GetIsClimbingLedge() &&
								   !bIsExhausted && CurrentState != Eps_Climbing && CurrentState != Eps_LeaveAiming && !bHasReachedWallWhileSprinting;
	if (SlideRequirements)
	{
		SetPlayerVelocity(FVector(0.f, 0.f, -250.f));
		GetCharacterMovement()->GravityScale = 0.f;
		bIsSlidingDown = true;
		MyAnimationComponent->SetCurrentAnimation(Ecmm_SlidingDown);
	}
	else
	{
		GetCharacterMovement()->GravityScale = GroundMovementData->GravityScale;
		bIsSlidingDown = false;
	}
}

void AMyCharacter::CheckExhaustion()
{
	if (MovementEnergy >= 1.f && bIsExhausted)
	{
		MyAnimationComponent->SetCurrentAnimation(Ecmm_Walking);
		bIsExhausted = false;
	}
	
	if (MovementEnergy <= 0.f && !bIsExhausted)
	{
		MyAnimationComponent->SetCurrentAnimation(Ecmm_Exhausted);
		bIsExhausted = true;
		if (CurrentState == Eps_Aiming)
			CurrentState = Eps_LeaveAiming;
		else if (CurrentState != Eps_Climbing)
			CurrentState = Eps_Walking;
	}

	MovementEnergy = FMath::Clamp(MovementEnergy, 0, 1);
}

void AMyCharacter::EnergyUsage()
{
	float MovementLoss = 0.f;

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
	
		if (ScaledFloorAngle > EnergyData->FloorAngleThreshold && !TraceDown)
		{
			bCanGainEnergy = false;
			UsingEnergy = true;
			MovementLoss = GetWorld()->DeltaTimeSeconds * EnergyData->RunningDepletionSpeed * ScaledFloorAngle;			
		}
		else
		{
			bCanGainEnergy = true;
			UsingEnergy = false;
		}
	}
	else if (CurrentState == Eps_Climbing)
	{
		UsingEnergy = true;
		MovementLoss = GetWorld()->DeltaTimeSeconds * EnergyData->ClimbingDepletionSpeed;
	}
	else
		UsingEnergy = false;

	if (ScaledFloorAngle < EnergyData->FloorAngleThreshold)
		bCanGainEnergy = true;
	
	if (MovementEnergy < 1.f && !UsingEnergy && bCanGainEnergy /*&& CurrentState != Eps_Climbing*/)
		MovementEnergy += GetWorld()->DeltaTimeSeconds * EnergyData->EnergyRegainSpeed;
	
	// Decide movement speed
	if (GetIsMidAir() && CurrentState != Eps_Climbing)
		return;
	
	// Flip 0 and 1 so movement speed is increased with higher value. 
	MovementSpeedPercent = FMath::Clamp(ScaledFloorAngle * - 1 + 1, 0, 2);

	MovementEnergy -= MovementLoss;
}

void AMyCharacter::HandleJumpInput()
{
	if (bIsExhausted)
		return;
	
	// Wall jumping + when running up wall
	if (bHasReachedWallWhileSprinting || bIsSlidingDown)
	{
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		
		constexpr int TraceLength = 100;
		const FVector TraceStart = GetActorLocation();
		const FVector TraceEnd = TraceStart + GetActorForwardVector() * TraceLength - GetActorUpVector() * GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const auto SurfaceTrace = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, BlockAllCollision, Params, FCollisionResponseParams());
	
		if (SurfaceTrace)
		{
			// Interrupt running
			FLatentActionManager LatentActionManager = GetWorld()->GetLatentActionManager();
			LatentActionManager.RemoveActionsForObject(this);

			// Jump in backward direction
			auto WallYawRotation = HitResult.ImpactNormal.Rotation().Yaw;

			SetActorRotation(FRotator(0, WallYawRotation, 0));
			SetPlayerVelocity(FVector(0.f, 0.f, GroundMovementData->WallJumpUpVelocity) + GetActorForwardVector() * GroundMovementData->WallJumpBackVelocity);
			bHasReachedWallWhileSprinting = false;
			MyAnimationComponent->SetCurrentAnimation(Ecmm_WallJumping);
			return;
		}
	}

	// Jump while climbing 
	if (CurrentState == Eps_Climbing && ClimbComponent->CheckCanClimb())
	{
		ClimbComponent->ResetCantClimbTimer();
		MovementEnergy -= EnergyData->JumpEnergyLoss;
		GetCharacterMovement()->BrakingDecelerationFlying = 1000.f;

		// Jump backward, straight out from wall 
		if (GetCharacterMovement()->Velocity.Length() == 0)
		{
			constexpr int RotationCorrection = 180;
			ClimbComponent->SetIsJumpingOutFromWall(true);
			MyAnimationComponent->SetCurrentAnimation(Ecmm_WallJumping);
			SetPlayerVelocity(FVector(0.f, 0.f, VelocityClimbJumpOutUp) - GetActorForwardVector() * VelocityClimbJumpOutBack);
			SetActorRotation(FRotator(0, GetActorRotation().Yaw + RotationCorrection, 0));
			return;
		}

		MyAnimationComponent->SetCurrentAnimation(Ecmm_ClimbJumping);
		
		// Jump in direction of movement input 
		SetPlayerVelocity(CharacterMovement * GroundMovementData->WallJumpUpVelocity);
		return;
	}

	if (GetCharacterMovement()->IsMovingOnGround())
	{
		MyAnimationComponent->SetCurrentAnimation(Ecmm_Jumping);
		MovementEnergy -= EnergyData->JumpEnergyLoss;
		GetCharacterMovement()->AddImpulse(FVector(0, 0, GroundMovementData->RegularJumpForce));
	}
}

void AMyCharacter::Landed(const FHitResult& Hit)
{
	Super::Super::Landed(Hit);

	if (!bIsExhausted)
		MyAnimationComponent->SetCurrentAnimation(CurrentState == Eps_Sprinting ? Ecmm_Sprinting : Ecmm_Walking);
	
	bHasReachedWallWhileSprinting = false;
}

void AMyCharacter::HandleSprintInput()
{
	if (CurrentState != Eps_Walking || bIsExhausted || GetCharacterMovement()->Velocity.Length() < 0.1f || bIsSlidingDown)
		return;
	
	CurrentState = Eps_Sprinting;
}

void AMyCharacter::HandleSprintStop()
{
	if (CurrentState != Eps_Sprinting || bHasReachedWallWhileSprinting)
		return;

	CurrentState = Eps_Walking;
}

void AMyCharacter::SetMovementSpeed(float TargetSpeed) const
{
	TargetSpeed = FMath::Clamp(TargetSpeed, 0, GroundMovementData->MaxSprintSpeed);
	const auto Alpha = TargetSpeed < GetCharacterMovement()->MaxWalkSpeed ? GroundMovementData->ReachTargetDownSpeed : GroundMovementData->ReachTargetUpSpeed;

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
	GroundMovementData->ObstacleTraceType,
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
	
	if (bShouldStopMovementOverTime || GetCharacterMovement()->MaxWalkSpeed < GroundMovementData->ThresholdToStopOverTime)
		return;
	
	bShouldStopMovementOverTime = true;
}

bool AMyCharacter::GetCanJumpBackwards() const
{
	return FloorAngle < GroundMovementData->ThresholdToJumpBack /*&& (GetActorForwardVector() - CharacterMovement).Length() > 1.7f*/;
}

void AMyCharacter::MovePlayer(const FVector& Destination, const bool EaseInOut, const float Duration)
{
	UKismetSystemLibrary::MoveComponentTo(
	RootComponent,
	Destination,
	FRotator(0, GetActorRotation().Yaw, 0),
	EaseInOut,
	EaseInOut,
	Duration,
	false,
	EMoveComponentAction::Move,
	LatentActionInfo
	);
}

bool AMyCharacter::GetIsMidAir() const
{
	return GetCharacterMovement()->Velocity.Z == 0 ? 0 : 1;
}

void AMyCharacter::CheckIfFalling()
{
	if (!bIsSlidingDown && GetCharacterMovement()->Velocity.Z < 0.f && CurrentState != Eps_Climbing && CurrentState != Eps_Aiming &&
		CurrentState != Eps_LeaveAiming && !bHasReachedWallWhileSprinting && !bIsExhausted && !ClimbComponent->GetIsClimbingLedge())
			MyAnimationComponent->SetCurrentAnimation(Ecmm_Jumping);
}

// Interrupt climbing
void AMyCharacter::CancelAction()
{
	if (CurrentState != Eps_Climbing)
		return;

	GetCharacterMovement()->MovementMode = MOVE_Walking;
	CurrentState = ClimbComponent->StopClimbing();
}

void AMyCharacter::CheckIdleness()
{
	TimeSinceMoved += GetWorld()->DeltaTimeSeconds;

	if (GetCharacterMovement()->Velocity.Length() != 0.f || GetCameraInput().Length() != 0.f)
		TimeSinceMoved = 0.f;
	
	// Time in seconds before perceived as idle 
	if (TimeSinceMoved > GroundMovementData->TimeBeforeIdle && CurrentState != Eps_Idle)
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
	ClimbComponent->BoostEnergy();
	CurrentState = HookshotComponent->UseHookshot(this);
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
	const FVector DistancedLookForWallEnd = GetActorLocation() + GetActorForwardVector() * GroundMovementData->DistanceBeforeAbleToRunUpWall;	
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
			GroundMovementData->ObstacleTraceType,
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
			MyAnimationComponent->SetCurrentAnimation(Ecmm_RunningUpWall);
			UKismetSystemLibrary::MoveComponentTo(
				RootComponent,
				RunningUpWallEndLocation,
				GetActorRotation(),
				true,
				false,
				GroundMovementData->RunningUpWallTimeInSeconds,
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

	MyAnimationComponent->SetCurrentAnimation(Ecmm_Walking);

	if (!GroundMovementData || !EnergyData)
		UE_LOG(LogTemp, Error, TEXT("MyCharacter.cpp - Data Asset missing!"))
}

void AMyCharacter::Tick(float const DeltaTime)
{
	Super::Tick(DeltaTime);
	
	MovementOutput();
	CheckFloorAngle();
	CheckIfFalling();

	/* Energy */
	CheckExhaustion();
	EnergyUsage();
	
	/* Climbing */
	CurrentState = ClimbComponent->FindClimbableWall();
	DecideIfShouldSlide();
	
	/* Running */
	RunUpToWall();
	RunningUpWall();

	// Keep last 
	PlayerStateSwitch();
}