#include "MyCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

AMyCharacter::AMyCharacter()
{
	CurrentCameraSpeed = StandardCameraSpeed;
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
			SpringArm->CameraLagSpeed = 20.f;
			SpringArm->CameraLagMaxDistance = 3.f;
			GetCharacterMovement()->bOrientRotationToMovement = true;
			break;
		case Eps_Sprinting:
			GetCharacterMovement()->SetWalkableFloorAngle(90.f);
			break;
		case Eps_Idle:
			break;
		case Eps_Aiming:
			CurrentCameraSpeed = AimCameraSpeed;
			SpringArm->CameraRotationLagSpeed = 1000.f;
			GetCharacterMovement()->bOrientRotationToMovement = false;
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
			GetCharacterMovement()->RotationRate = FRotator(0.f, AimRotationRate, 0.f);
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.05f);
			break;
		case Eps_LeaveAiming:
			SpringArm->CameraRotationLagSpeed = 50.f;
			CurrentCameraSpeed = StandardCameraSpeed;
			GetCharacterMovement()->RotationRate = FRotator(0.f, StandardRotationRate, 0.f);
			GetCharacterMovement()->bOrientRotationToMovement = true;
			GetCharacterMovement()->bUseControllerDesiredRotation = false;
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
			break;
		case Eps_Climbing:
			GetCharacterMovement()->MovementMode = MOVE_Flying;
			GetCharacterMovement()->bOrientRotationToMovement = false;
			SpringArm->CameraLagSpeed = 1.f;
			SpringArm->CameraLagMaxDistance = 10.f;
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("No active Player State. Now Walking"))
		}	
	}
	PreviousState = CurrentState;

	// Checks state every tick  
	switch (CurrentState)
	{		
	case Eps_Walking:
		CheckFloorAngle();
		CheckIdleness();
		GetCharacterMovement()->MaxWalkSpeed = 600.f * MovementSpeedPercent * MovementEnergy;
		SpringArm->TargetArmLength = FMath::Lerp(SpringArm->TargetArmLength, StandardSpringArmLength, NormalCameraSwitchSpeed);
		if (GetCharacterMovement()->Velocity.Length() != 0)
			FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, WalkingFOV, NormalCameraSwitchSpeed/4);
		else
			FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, StillFOV, NormalCameraSwitchSpeed);
		break;
	case Eps_Sprinting:
		CheckFloorAngle();
		GetCharacterMovement()->MaxWalkSpeed = 1000.f * MovementSpeedPercent * MovementEnergy;
		SpringArm->TargetArmLength = FMath::Lerp(SpringArm->TargetArmLength, SprintingSpringArmLength, SpringArmSwitchSpeed);
		FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, SprintingFOV, SprintFOVSpeed);
		if (GetCharacterMovement()->Velocity.Length() < 0.1f)
			HandleSprintStop();
		break;
	case Eps_Idle:
		CheckIdleness();
		AddControllerYawInput(GetWorld()->DeltaTimeSeconds * 2);
		SpringArm->TargetArmLength = FMath::Lerp(SpringArm->TargetArmLength, 10000.f, 0.0001f);
		FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, 60.f, 0.0001f);
		if (SpringArm->GetTargetRotation().Vector().Z > -0.25f)
			AddControllerPitchInput(GetWorld()->DeltaTimeSeconds / 2);
		else if (SpringArm->GetTargetRotation().Vector().Z < -0.3f)
			AddControllerPitchInput(-GetWorld()->DeltaTimeSeconds / 2);
		
		if (TimeSinceMoved < 2.f)
			CurrentState = SavedState;
		break;		
	case Eps_Aiming:
		MovementEnergy -= GetWorld()->DeltaTimeSeconds * AimEnergyDepletionSpeed;
		SpringArm->TargetArmLength = FMath::Lerp(SpringArm->TargetArmLength, 100.f, 0.3f);
		SpringArm->SocketOffset = FMath::Lerp(SpringArm->SocketOffset, AimingCameraOffset, AimingCameraTransitionAlpha);
		FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, AimingFOV, AimingCameraTransitionAlpha);
		break;
	case Eps_LeaveAiming:
		SpringArm->TargetArmLength = FMath::Lerp(SpringArm->TargetArmLength, StopAimingSpringArmLength, NormalCameraSwitchSpeed);
		FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, WalkingFOV, NormalCameraSwitchSpeed);
		if (bIsUsingHookshot && (GetActorLocation() - TargetLocation).Length() < 10.f)
			bIsUsingHookshot = false;

		if (!bIsUsingHookshot && SavedState == Eps_Sprinting)
			CurrentState = Eps_Walking;
		else if (!bIsUsingHookshot)
			CurrentState = SavedState;
		break;
	case Eps_Climbing:
		SpringArm->TargetArmLength = FMath::Lerp(SpringArm->TargetArmLength, StandardSpringArmLength, NormalCameraSwitchSpeed);
		FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, WalkingFOV, NormalCameraSwitchSpeed);
		// Makes sure that the player is pushed to the wall and doesn't fall off
		if (CantClimbTimer >= 1.f)
			GetCharacterMovement()->AddImpulse(GetActorForwardVector() * 1000.f);
		break;		
	default:
		UE_LOG(LogTemp, Error, TEXT("No active Player State. Now Walking"))
		CurrentState = Eps_Walking;
		break;
	}
			
	if (bIsUsingHookshot || bHasReachedWallWhileSprinting)
		SetPlayerVelocity(FVector(0, 0, 0));
	
	if (CurrentState != Eps_Climbing)
		CurrentCameraOffsetZ = FMath::Lerp(CurrentCameraOffsetZ, 0.f, NormalCameraSwitchSpeed);
	
	if (CurrentState != Eps_Aiming)
		SpringArm->SocketOffset = FMath::Lerp(SpringArm->SocketOffset, FVector(
			0.f, CurrentCameraOffsetY, CurrentCameraOffsetZ), NormalCameraSwitchSpeed);

	if (CurrentState != Eps_Idle)
		FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, WalkingFOV, 0.001f);
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

	if (CurrentState != Eps_Climbing)
	{
		// Walk toward camera's forward vector 
		const FRotator CameraRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, CameraRotation.Yaw, 0.f);
		CharacterMovement = YawRotation.RotateVector(CharacterMovement);
	}
	else
	{
		// Climb in relation to character's rotation 
		CharacterMovement = GetActorForwardVector().Rotation().RotateVector(CharacterMovement);
	}
	
	AddMovementInput(CharacterMovement);
}

void AMyCharacter::SetPlayerVelocity(const FVector& Value) const
{
	GetCharacterMovement()->Velocity = Value;
}

void AMyCharacter::CheckFloorAngle()
{
	const FVector StartFloorAngle = GetActorLocation();
	const FVector EndForwardAngle = GetActorLocation() + GetActorForwardVector() * 150.f - GetActorUpVector() * 300.f;
	const FVector EndRightAngle = GetActorLocation() + GetActorForwardVector() * 150.f - GetActorUpVector() * 300.f + GetActorRightVector() * 100.f;
	const FVector EndLeftAngle = GetActorLocation() + GetActorForwardVector() * 150.f - GetActorUpVector() * 300.f - GetActorRightVector() * 100.f;

	FHitResult HitResultForward;
	FHitResult HitResultRight;
	FHitResult HitResultLeft;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);

	const auto ForwardTrace = GetWorld()->LineTraceSingleByChannel(HitResultForward, StartFloorAngle, EndForwardAngle, BlockAllCollision, Parameters, FCollisionResponseParams());
	const auto RightTrace = GetWorld()->LineTraceSingleByChannel(HitResultRight, StartFloorAngle, EndRightAngle, BlockAllCollision, Parameters, FCollisionResponseParams());
	const auto LeftTrace = GetWorld()->LineTraceSingleByChannel(HitResultLeft, StartFloorAngle, EndLeftAngle, BlockAllCollision, Parameters, FCollisionResponseParams());

	// If the line trace is null distance = 0. Then setting distance to max so the other ones are shorter. 
	if (!ForwardTrace)
		HitResultForward.Distance = FLT_MAX;		
	if (!RightTrace)
		HitResultRight.Distance = FLT_MAX;	
	if (!LeftTrace)
		HitResultLeft.Distance = FLT_MAX;

	// Get the shortest trace
	FloorAngle = FMath::Min3(HitResultForward.Distance, HitResultLeft.Distance, HitResultRight.Distance) / 100.f;

	if (FloorAngle < 0.77f && GetCharacterMovement()->Velocity.Z < 0.f && !bIsExhausted && CurrentState != Eps_Climbing)
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
	
	if (MovementEnergy >= 1.f)
		bIsExhausted = false;
	
	if (MovementEnergy <= 0.f && !bIsExhausted)
	{
		bIsExhausted = true;
		CurrentState = CurrentState == Eps_Aiming ? Eps_LeaveAiming : Eps_Walking;
	}

	const auto CorrectedFloorAngle = -FloorAngle + 1.007908f;
	const auto ScaledFloorAngle = UKismetMathLibrary::NormalizeToRange(CorrectedFloorAngle, 0, 0.4f);
	bool UsingEnergy;
	
	// UE_LOG(LogTemp, Warning, TEXT("Corrected floor: %f | Scaled floor: %f | Floor angle: %f"), CorrectedFloorAngle, ScaledFloorAngle, FloorAngle);
	
	if (MovementEnergy > 0.f &&
		GetCharacterMovement()->Velocity.Length() > 0.f &&
		CurrentState == Eps_Sprinting)
	{
		if (ScaledFloorAngle > 0.6f)
		{
			UsingEnergy = true;
			MovementLoss = GetWorld()->DeltaTimeSeconds * ExhaustionSpeed * ScaledFloorAngle;			
		}
		else
			UsingEnergy = false;
	}
	else
		UsingEnergy = false;
	
	if (MovementEnergy < 1.f && !UsingEnergy)
		MovementEnergy += GetWorld()->DeltaTimeSeconds * ExhaustionSpeed;
	
	// Decide movement speed
	if (GetIsMidAir())
		return;

	// TO DO: Can this be removed? 
	if (FloorAngle == 0.f)
		FloorAngle = 1.f;
	else if (FloorAngle < 0.8f)
		FloorAngle = FloorAngle * FloorAngle * FloorAngle;	
	else if (FloorAngle < 1.f)
		FloorAngle *= FloorAngle;

	// Flip 0 and 1 so movement speed is increased with higher value. 
	MovementSpeedPercent = FMath::Clamp(ScaledFloorAngle * - 1 + 1, 0, 2);

	MovementEnergy -= MovementLoss;
}

void AMyCharacter::HandleJumpInput()
{
	// When running up wall 
	if (CurrentState != Eps_LeaveAiming && FloorAngle < 2.f && bHasReachedWallWhileSprinting)
	{
		// Interrupt running
		FLatentActionManager LatentActionManager = GetWorld()->GetLatentActionManager();
		LatentActionManager.RemoveActionsForObject(this);

		// Jump in backward direction 
		GetCharacterMovement()->AddImpulse(FVector(0.f, 0.f, JumpImpulseUp) - GetActorForwardVector() * JumpImpulseBack);
		bHasReachedWallWhileSprinting = false;
		return;
	}

	// Jump while climbing 
	if (CurrentState == Eps_Climbing && MovementEnergy > 0.f && CantClimbTimer >= 1.f)
	{
		CantClimbTimer = 0.5f;
		MovementEnergy -= 0.3f;
		GetCharacterMovement()->BrakingDecelerationFlying = 1000.f;

		// Jump backward, straight out from wall 
		if (GetCharacterMovement()->Velocity.Length() == 0)
		{
			bIsJumpingOutFromWall = true;
			// GetCharacterMovement()->Velocity = FVector(0.f, 0.f, VelocityClimbJumpOutUp) - GetActorForwardVector() * VelocityClimbJumpOutBack;
			GetCharacterMovement()->AddImpulse(FVector(0.f, 0.f, ClimbJumpOutImpulseUp) - GetActorForwardVector() * ClimbJumpOutImpulseBack);
			SetActorRotation(FRotator(0, GetActorRotation().Yaw + 180, 0));
			return;
		}

		// Jump in direction of movement input 
		GetCharacterMovement()->AddImpulse(CharacterMovement * JumpImpulseUp);
		return;
	}
	
	if (BCanJumpBackwards() && !GetCharacterMovement()->IsMovingOnGround())
		GetCharacterMovement()->AddImpulse(FVector(0.f, 0.f, JumpImpulseUp) + CharacterMovement * JumpImpulseBack);
	else if (BCanJumpBackwards() && GetCharacterMovement()->IsMovingOnGround())
		GetCharacterMovement()->AddImpulse(CharacterMovement * JumpImpulseBack);
	
	if (GetCharacterMovement()->IsMovingOnGround() && !bIsExhausted)
	{
		MovementEnergy -= 0.3f;
		Jump();
	}
}

void AMyCharacter::Landed(const FHitResult& Hit)
{
	Super::Super::Landed(Hit);

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
	if (CurrentState != Eps_Sprinting)
		return;

	CurrentState = Eps_Walking;
}

bool AMyCharacter::BCanJumpBackwards() const
{
	if (FloorAngle < 0.77f && (GetActorForwardVector() - CharacterMovement).Length() > 1.7f)
		return true;
	return false;
}

bool AMyCharacter::GetIsMidAir() const
{
	return GetCharacterMovement()->Velocity.Z == 0 ? 0 : 1;
}

void AMyCharacter::FindClimbableWall()
{
	if (CantClimbTimer < 1.f)
	{
		CantClimbTimer += GetWorld()->DeltaTimeSeconds;
		FindClimbRotation();
		return;
	}

	const auto World = GetWorld();
	const auto ActorLocation = GetActorLocation();
	const auto ForwardVector = GetActorForwardVector();
	const auto RightVector = GetActorRightVector();
	
	const FVector StartWallAngle = ActorLocation;
	const FVector EndForwardAngle = ActorLocation + ForwardVector * 80.f;
	const FVector EndRightAngle = ActorLocation + ForwardVector * 80.f + RightVector * ClimbingSensitivityWidth;
	const FVector EndLeftAngle = ActorLocation + ForwardVector * 80.f - RightVector * ClimbingSensitivityWidth;

	FHitResult HitResultForward;
	FHitResult HitResultRight;
	FHitResult HitResultLeft;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);

	const auto ForwardTrace = World->LineTraceSingleByChannel(HitResultForward, StartWallAngle, EndForwardAngle, ClimbingCollision, Parameters, FCollisionResponseParams());
	const auto RightTrace = World->LineTraceSingleByChannel(HitResultRight, StartWallAngle, EndRightAngle, ClimbingCollision, Parameters, FCollisionResponseParams());
	const auto LeftTrace = World->LineTraceSingleByChannel(HitResultLeft, StartWallAngle, EndLeftAngle, ClimbingCollision, Parameters, FCollisionResponseParams());

	// Check if LineTraces find the climbing wall. 
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

	const auto World = GetWorld();
	const auto ActorLocation = GetActorLocation();
	const auto ForwardVector = GetActorForwardVector();
	const auto RightVector = GetActorRightVector();
	const auto UpVector = GetActorUpVector();
	const auto CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius() * 0.5f;
	const auto MovementSideways = GetMovementSideways();
	const auto MovementForward = GetMovementForward();
	
	const FVector StartBackWallDetection = ActorLocation + ForwardVector * CapsuleRadius;
	const FVector EndBackWallDetection = ActorLocation + UpVector * MovementForward * 4000 + RightVector * MovementSideways * 4000 - ForwardVector * 30.f;
	auto WallRotationTrace = World->LineTraceSingleByChannel(HitResultPlayerRotation, StartBackWallDetection, EndBackWallDetection, ClimbingCollision, Parameters, FCollisionResponseParams());
	
	if (!WallRotationTrace)
	{
		const FVector StartSideWallDetection = ActorLocation;
		const FVector EndSideWallDetection = ActorLocation + MovementSideways * RightVector * 5000.f;
		WallRotationTrace = World->LineTraceSingleByChannel(HitResultPlayerRotation, StartSideWallDetection, EndSideWallDetection, ClimbingCollision, Parameters, FCollisionResponseParams());
		if (WallRotationTrace)
			goto setrot;
	
		const FVector StartFrontWallDetection = ActorLocation - ForwardVector * CapsuleRadius;
		const FVector EndFrontWallDetection = ActorLocation + UpVector * MovementForward * AdjustPlayerRotationDistance + RightVector * MovementSideways * AdjustPlayerRotationDistance + ForwardVector * 80.f;
		WallRotationTrace = World->LineTraceSingleByChannel(HitResultPlayerRotation, StartFrontWallDetection, EndFrontWallDetection, ClimbingCollision, Parameters, FCollisionResponseParams());
		if (WallRotationTrace)
			goto setrot;
	
		const FVector EndEyesightWallDetection = ActorLocation + ForwardVector * 100.f;
		WallRotationTrace = World->LineTraceSingleByChannel(HitResultPlayerRotation, ActorLocation, EndEyesightWallDetection, ClimbingCollision, Parameters, FCollisionResponseParams());
	}
	
	if (WallRotationTrace)
	{
		setrot:

		if (bIsJumpingOutFromWall && CurrentClimbingWall == HitResultPlayerRotation.GetActor())
			return;

		bIsJumpingOutFromWall = false;
		CurrentClimbingWall = HitResultPlayerRotation.GetActor();
		SetPlayerRotation(HitResultPlayerRotation.ImpactNormal.Rotation());
	}
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
	const FVector LowTraceStart = GetActorLocation();
	const FVector LowTraceEnd = LowTraceStart + GetActorForwardVector() * LedgeClimbDetectionOffset.X;
	const FVector HighTraceStart = GetActorLocation() + GetActorUpVector() * LedgeClimbDetectionOffset.Z;
	const FVector HighTraceEnd = HighTraceStart + GetActorForwardVector() * LedgeClimbDetectionOffset.X;
	
	FHitResult LowHitResult;
	FHitResult HighHitResult;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);
	const auto LowTrace = World->LineTraceSingleByChannel(LowHitResult, LowTraceStart, LowTraceEnd, LedgeChannel, Parameters, FCollisionResponseParams());
	const auto HighTrace = World->LineTraceSingleByChannel(HighHitResult, HighTraceStart, HighTraceEnd, BlockAllCollision, Parameters, FCollisionResponseParams());

	if (LowTrace && !HighTrace)
	{
		LedgeClimbDestination = HighTraceEnd + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		bIsClimbingLedge = true;
		
		UKismetSystemLibrary::MoveComponentTo(
			RootComponent,
			LedgeClimbDestination,
			GetActorRotation()/*LowHitResult.ImpactNormal.Rotation().GetInverse()*/,
			true,
			true,
			LedgeClimbDuration,
			false,
			EMoveComponentAction::Move,
			LatentActionInfo
			);
	}
}

void AMyCharacter::CameraMovementOutput()
{
	if (Controller == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter Line 125, no controller"));
		return;
	}
	
	CameraMovement = FVector2D(GetCameraMovementX(), GetCameraMovementY());

	// Rotation
	AddControllerYawInput(CameraMovement.X * CurrentCameraSpeed * GetWorld()->DeltaTimeSeconds);
	AddControllerPitchInput(-CameraMovement.Y * CurrentCameraSpeed * GetWorld()->DeltaTimeSeconds);

	// Vector offset 
	if (GetCharacterMovement()->Velocity.Length() > 0.1f)
	{
		if ((CharacterMovement.Rotation().Vector() - FollowCamera->GetRightVector()).Length() > 1.8f)
				SetCurrentOffset(CurrentCameraOffsetY, -CameraYDirectionSpeed, CameraClamp.Y);

		else if ((CharacterMovement.Rotation().Vector() - FollowCamera->GetRightVector()).Length() < 0.8f)
				SetCurrentOffset(CurrentCameraOffsetY, CameraYDirectionSpeed, CameraClamp.Y);

		if (CurrentState == Eps_Climbing
		&& (CharacterMovement.Rotation().Vector() - FollowCamera->GetUpVector()).Length() > 1.8f)
			SetCurrentOffset(CurrentCameraOffsetZ, -CameraYDirectionSpeed, CameraClamp.Z);
	
		else if (CurrentState == Eps_Climbing
		&& (CharacterMovement.Rotation().Vector() - FollowCamera->GetUpVector()).Length() < 0.8f)
			SetCurrentOffset(CurrentCameraOffsetZ, CameraYDirectionSpeed, CameraClamp.Z);
	}

	float CameraSpeed;	
	if (CurrentState == Eps_Climbing)
	{
		CameraSpeed = CameraYDirectionSpeed * 2;
		if (CameraMovement.Y < -0.005f)
				SetCurrentOffset(CurrentCameraOffsetZ, -CameraSpeed, CameraClamp.Z);

		else if (CameraMovement.Y > 0.005f)
				SetCurrentOffset(CurrentCameraOffsetZ, CameraSpeed, CameraClamp.Z);
	}
	else
	{
		CameraSpeed = CameraYDirectionSpeed / (CharacterMovement.Length() + 1);
		if (CameraMovement.X < -0.005f)
				SetCurrentOffset(CurrentCameraOffsetY, -CameraSpeed, CameraClamp.Y);

		else if (CameraMovement.X > 0.005f)
				SetCurrentOffset(CurrentCameraOffsetY, CameraSpeed, CameraClamp.Y);
	}
}

void AMyCharacter::CheckIdleness()
{
	TimeSinceMoved += GetWorld()->DeltaTimeSeconds;

	if (GetCharacterMovement()->Velocity.Length() != 0.f || CameraMovement.Length() != 0.f)
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
	if (CurrentState == Eps_Idle
		|| GetCharacterMovement()->IsMovingOnGround()
		|| bIsExhausted)
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

void AMyCharacter::SetCurrentOffset(float& Value, const float Speed, const float Clamp) const
{
	Value += Speed * GetWorld()->DeltaTimeSeconds;
	Value = FMath::Clamp(Value, -Clamp, Clamp);
}

// Look for target to hook to with hookshot
void AMyCharacter::HandleActionInput()
{
	if (CurrentState != Eps_Aiming)
		return;
	
	FHitResult HookshotTarget;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);

	const FVector StartHookSearch = FollowCamera->GetComponentLocation();
	const FVector EndHookSearch = FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector() * HookLength;

	DrawDebugLine(GetWorld(), StartHookSearch, EndHookSearch, FColor::Purple, false, 0.1f, 0.f, 3.f);

	const auto HookTrace = GetWorld()->LineTraceSingleByChannel(HookshotTarget, StartHookSearch, EndHookSearch, HookCollision, Parameters, FCollisionResponseParams());
	if (HookTrace)
	{
		TargetLocation = HookshotTarget.Location - FollowCamera->GetForwardVector() * 50.f - FVector(0, 0, 100.f);
		bIsUsingHookshot = true;
		CurrentState = Eps_LeaveAiming;
		
		UKismetSystemLibrary::MoveComponentTo(
			RootComponent,
			TargetLocation,
			FRotator(0, FollowCamera->GetForwardVector().Rotation().Yaw, 0),
			false,
			false,
			HookshotTarget.Distance/HookshotSpeed,
			false,
			EMoveComponentAction::Move,
			LatentActionInfo
			);
	}
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

	FHitResult DistancedLookForWallHitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	const FVector TraceStart = GetActorLocation();
	const FVector DistancedLookForWallEnd = GetActorLocation() + GetActorForwardVector() * DistanceBeforeAbleToRunUpWall;	
	const auto DistancedLookForWall = GetWorld()->LineTraceSingleByChannel(DistancedLookForWallHitResult, TraceStart, DistancedLookForWallEnd, BlockAllCollision, Params, FCollisionResponseParams());

	if (!DistancedLookForWall)
		bIsNearingWall = false;
	
	FHitResult DistanceCapsuleHitResult;
	const FVector DistancedTraceEnd = DistancedLookForWallEnd - GetActorForwardVector() * 50;

	const auto DistancedCapsuleTrace = UKismetSystemLibrary::CapsuleTraceSingle(
		GetWorld(),
		TraceStart,
		DistancedTraceEnd,
		GetCapsuleComponent()->GetScaledCapsuleRadius(),
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		ObstacleTraceType,
		false,
		{this},
		EDrawDebugTrace::None,
		DistanceCapsuleHitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		1.f
		);

	if (DistancedLookForWall && !DistancedCapsuleTrace)
		bIsNearingWall = true;
	
	if (!bIsNearingWall)
		return;
	
	FHitResult ShortWallSearchHitResult;
	const FVector TraceEnd = GetActorLocation() + GetActorForwardVector() * 40.f;
	
	const auto LookForWall = GetWorld()->LineTraceSingleByChannel(ShortWallSearchHitResult, TraceStart, TraceEnd, BlockAllCollision, Params, FCollisionResponseParams());
	if (LookForWall)
	{
		FHitResult CapsuleHitResult;
		RunningUpWallEndLocation = GetActorLocation() + GetActorUpVector() * 200.f;
		
		auto CapsuleTrace = UKismetSystemLibrary::CapsuleTraceSingle(
			GetWorld(),
			TraceStart,
			RunningUpWallEndLocation,
			GetCapsuleComponent()->GetScaledCapsuleRadius(),
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
			ObstacleTraceType,
			false,
			{this},
			EDrawDebugTrace::ForDuration,
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
						
			UKismetSystemLibrary::MoveComponentTo(
				RootComponent,
				RunningUpWallEndLocation,
				GetActorRotation(),
				true,
				false,
				RunningUpWallSpeed,
				false,
				EMoveComponentAction::Move,
				LatentActionInfo
				);
		}
	}
}

void AMyCharacter::RunningUpWall()
{			
	if (bHasReachedWallWhileSprinting)
		SetPlayerVelocity(FVector(0, 0, 0));
	if (UKismetMathLibrary::Vector_Distance(GetActorLocation(), RunningUpWallEndLocation) < 10.f)
		bHasReachedWallWhileSprinting = false;
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMyCharacter::Tick(float const DeltaTime)
{
	Super::Tick(DeltaTime);

	MovementOutput();
	CameraMovementOutput();
	CheckFloorAngle();
	
	/* Climbing */
	CheckExhaustion();
	FindClimbableWall();
	LookForLedge();
	
	/* Running */
	RunUpToWall();
	RunningUpWall();

	// Keep last 
	PlayerStateSwitch();

	// UE_LOG(LogTemp, Warning, TEXT("Player velocity: %s"), *GetCharacterMovement()->Velocity.ToString())
	// UE_LOG(LogTemp, Warning, TEXT("Saved state: %d"), SavedState.GetValue())
	// UE_LOG(LogTemp, Warning, TEXT("Player state: %d"), CurrentState.GetValue())
	// UE_LOG(LogTemp, Warning, TEXT("FoundWall: %hhd"), bHasReachedWallWhileSprinting)
}