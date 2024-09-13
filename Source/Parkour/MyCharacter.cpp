#include "MyCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 50.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
	FollowCamera->SetupAttachment(SpringArm);
	FollowCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->AirControl = 0.3f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CurrentCameraSpeed = StandardCameraSpeed;
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
	CheckExhaustion();
	CheckBackwardJumpEligibility();
	CheckWallClimb();

	// UE_LOG(LogTemp, Warning, TEXT("Player velocity: %s"), *GetCharacterMovement()->Velocity.ToString())
	// UE_LOG(LogTemp, Warning, TEXT("Saved state: %d"), SavedState.GetValue())
	// UE_LOG(LogTemp, Warning, TEXT("Player state: %d"), CurrentState.GetValue())
	
	switch (CurrentState)
	{
	case Eps_Walking:
		GetCharacterMovement()->MaxWalkSpeed = 600.f * MovementSpeedPercent * MovementEnergy;
		GetCharacterMovement()->SetWalkableFloorAngle(50.f);
		GetCharacterMovement()->bOrientRotationToMovement = true;
		CheckFloorAngle();
		CheckIdleness();
		SetSpringArmLength(StandardSpringArmLength, SpringArmSwitchSpeed);
		break;

	case Eps_Sprinting:
		GetCharacterMovement()->MaxWalkSpeed = 1000.f * MovementSpeedPercent * MovementEnergy;
		GetCharacterMovement()->SetWalkableFloorAngle(90.f);
		CheckFloorAngle();
		SetSpringArmLength(SprintingSpringArmLength, SpringArmSwitchSpeed);
		break;
		
	case Eps_Aiming:
		CurrentCameraSpeed = AimCameraSpeed;
		SpringArm->CameraRotationLagSpeed = 1000.f;
		SetSpringArmLength(100.f, 0.3f);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, AimRotationRate, 0.f);
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.05f);
		SetSpringArmOffset(FVector(0.f, 50.f, -10.f), 0.3f);
		break;

	case Eps_LeaveAiming:
		SpringArm->CameraRotationLagSpeed = 50.f;
		CurrentCameraSpeed = StandardCameraSpeed;
		GetCharacterMovement()->RotationRate = FRotator(0.f, StandardRotationRate, 0.f);
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
		
		if (bIsUsingHookShot)
		{
			GetCharacterMovement()->Velocity = FVector(0, 0, 0);
			SetSpringArmLength(300.f, SpringArmSwitchSpeed);
		}

		if (bIsUsingHookShot && (GetActorLocation() - TargetLocation).Length() < 10.f)
			bIsUsingHookShot = false;

		if (!bIsUsingHookShot && SavedState == Eps_Sprinting)
			CurrentState = Eps_Walking;
		
		else if (!bIsUsingHookShot)
			CurrentState = SavedState;
		break;

	case Eps_Idle:
		CheckIdleness();
		AddControllerYawInput(GetWorld()->DeltaTimeSeconds * 2);
		SetSpringArmLength(1000.f, 0.0001f);
		FollowCamera->FieldOfView = FMathf::Lerp(FollowCamera->FieldOfView, 105, 0.001f);
		
		if (SpringArm->GetTargetRotation().Vector().Z > -0.25f)
			AddControllerPitchInput(GetWorld()->DeltaTimeSeconds / 2);
			
		else if (SpringArm->GetTargetRotation().Vector().Z < -0.3f)
			AddControllerPitchInput(-GetWorld()->DeltaTimeSeconds / 2);
		
		if (TimeSinceMoved < 2.f)
			CurrentState = SavedState;
		break;

	case Eps_Climbing:
		// UE_LOG(LogTemp, Warning, TEXT("Climbing."))
		GetCharacterMovement()->MovementMode = MOVE_Flying;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		
		// Makes sure that the player is pushed to the wall and doesn't fall off
		if (CantClimbTimer >= 1.f)
			GetCharacterMovement()->AddImpulse(GetActorForwardVector() * 1000.f);
		break;
		
	default:
		UE_LOG(LogTemp, Error, TEXT("No active Player State. Now Walking"))
		CurrentState = Eps_Walking;
		break;
	}

	if (CurrentState != Eps_Aiming)
	{
		SetSpringArmOffset(FVector(0.f, 0.f, 0.f), SpringArmSwitchSpeed);
	}

	if (CurrentState != Eps_Idle)
		FollowCamera->FieldOfView = FMathf::Lerp(FollowCamera->FieldOfView, 90, 0.001f);
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Forwards", this, &AMyCharacter::HandleForwardInput);
	PlayerInputComponent->BindAxis("Sideways", this, &AMyCharacter::HandleSidewaysInput);
	PlayerInputComponent->BindAxis("MouseX", this, &AMyCharacter::HandleMouseInputX);
	PlayerInputComponent->BindAxis("MouseY", this, &AMyCharacter::HandleMouseInputY);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::HandleJumpInput);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyCharacter::HandleSprintInput);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyCharacter::HandleSprintStop);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMyCharacter::HandleAimInput);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMyCharacter::HandleAimStop);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyCharacter::LookForHook);
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AMyCharacter::StopClimbing);
}

void AMyCharacter::HandleForwardInput(const float Value)
{
	CharacterMovementForward = Value * GetWorld()->DeltaTimeSeconds;
}

void AMyCharacter::HandleSidewaysInput(const float Value)
{
	CharacterMovementSideways = Value * GetWorld()->DeltaTimeSeconds;
}

void AMyCharacter::MovementOutput()
{
	if (Controller == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter Line 99, no controller"));
		return;
	}

	if (CurrentState != Eps_Climbing)
		CharacterMovement = FVector(CharacterMovementForward, CharacterMovementSideways, 0.f);

	else
		CharacterMovement = FVector(0.f, CharacterMovementSideways, CharacterMovementForward);
	
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

void AMyCharacter::CheckFloorAngle()
{
	FVector StartFloorAngle = GetActorLocation();
	FVector EndForwardAngle = GetActorLocation() + GetActorForwardVector() * 150.f - GetActorUpVector() * 300.f;
	FVector EndRightAngle = GetActorLocation() + GetActorForwardVector() * 150.f - GetActorUpVector() * 300.f + GetActorRightVector() * 100.f;
	FVector EndLeftAngle = GetActorLocation() + GetActorForwardVector() * 150.f - GetActorUpVector() * 300.f - GetActorRightVector() * 100.f;

	FHitResult HitResultForward;
	FHitResult HitResultRight;
	FHitResult HitResultLeft;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);
	
	// DrawDebugLine(GetWorld(), StartFloorAngle, EndForwardAngle, FColor::Red, false, 0.5f, 0.f, 2.f);
	// DrawDebugLine(GetWorld(), StartFloorAngle, EndRightAngle, FColor::Green, false, 0.5f, 0.f, 2.f);
	// DrawDebugLine(GetWorld(), StartFloorAngle, EndLeftAngle, FColor::Blue, false, 0.5f, 0.f, 2.f);

	// If the line trace is null distance = 0. Then setting distance to max so the other ones are shorter. 
	if (!GetWorld()->LineTraceSingleByChannel(HitResultForward, StartFloorAngle, EndForwardAngle, ECC_GameTraceChannel18, Parameters, FCollisionResponseParams()))
		HitResultForward.Distance = FLT_MAX;
	
	if (!GetWorld()->LineTraceSingleByChannel(HitResultRight, StartFloorAngle, EndRightAngle, ECC_GameTraceChannel18, Parameters, FCollisionResponseParams()))
		HitResultRight.Distance = FLT_MAX;
	
	if (!GetWorld()->LineTraceSingleByChannel(HitResultLeft, StartFloorAngle, EndLeftAngle, ECC_GameTraceChannel18, Parameters, FCollisionResponseParams()))
		HitResultLeft.Distance = FLT_MAX;

	FloorAngle = FMath::Min3(HitResultForward.Distance, HitResultLeft.Distance, HitResultRight.Distance) / 100.f;
}

void AMyCharacter::CheckExhaustion()
{
	if (MovementEnergy >= 1.f)
		bIsExhausted = false;
	
	if (MovementEnergy <= 0.f && !bIsExhausted)
	{
		bIsExhausted = true;
		CurrentState = Eps_Walking;
	}
	
	else if (FloorAngle < 0.74f
		&& MovementEnergy > 0.f
		&& GetCharacterMovement()->Velocity.Length() > 0.f
		&& CurrentState == Eps_Sprinting)
		MovementEnergy -= GetWorld()->DeltaTimeSeconds * ExhaustionSpeed;
		
	else if (FloorAngle < 0.77f
		&& MovementEnergy > 0.f
		&& GetCharacterMovement()->Velocity.Length() > 0.f
		&& CurrentState == Eps_Sprinting)
		MovementEnergy -= GetWorld()->DeltaTimeSeconds * ExhaustionSpeed / 2;
	
	else if (FloorAngle < 0.8f
		&& MovementEnergy > 0.f
		&& GetCharacterMovement()->Velocity.Length() > 0.f
		&& CurrentState == Eps_Sprinting)
		MovementEnergy -= GetWorld()->DeltaTimeSeconds * ExhaustionSpeed / 4;

	// I just wanted some exponential energy drain depending on the floor angle. Above is better though
	// else if (FloorAngle < 0.8f
	// 	&& MovementEnergy > 0.f
	// 	&& GetCharacterMovement()->Velocity.Length() > 0.f
	// 	&& CurrentState == Eps_Sprinting)
	// 	MovementEnergy -= GetWorld()->DeltaTimeSeconds * ExhaustionSpeed / FMath::Pow(10, FloorAngle) * 3;
	//
	else if (MovementEnergy < 1.f)
		MovementEnergy += GetWorld()->DeltaTimeSeconds * ExhaustionSpeed;
	
	// Decide movement speed
	if (FloorAngle == 0.f)
		FloorAngle = 1.f;

	else if (FloorAngle < 0.8f)
		FloorAngle = FloorAngle * FloorAngle * FloorAngle;
	
	else if (FloorAngle < 1.f)
		FloorAngle *= FloorAngle;
	
	MovementSpeedPercent = FloorAngle;
}

void AMyCharacter::HandleJumpInput()
{
	if (CurrentState == Eps_Climbing)
	{
		CantClimbTimer = 0.5f;
		MovementEnergy -= 0.3f;
		GetCharacterMovement()->BrakingDecelerationFlying = 1000.f;
		GetCharacterMovement()->AddImpulse(CharacterMovement * JumpImpulseUp);
		UE_LOG(LogTemp, Warning, TEXT("Wall jumping. Movement: %s"), *CharacterMovement.ToString());
		return;
	}
	
	if (bHasBackwardJumpAngle && bIsTurningBackward && !GetCharacterMovement()->IsMovingOnGround())
		GetCharacterMovement()->AddImpulse(FVector(0.f, 0.f, JumpImpulseUp) + CharacterMovement * JumpImpulseBack);
	
	else if (bHasBackwardJumpAngle && bIsTurningBackward && GetCharacterMovement()->IsMovingOnGround())
		GetCharacterMovement()->AddImpulse(CharacterMovement * JumpImpulseBack);
	
	if (GetCharacterMovement()->IsMovingOnGround() && !bIsExhausted)
	{
		MovementEnergy -= 0.3f;
		Jump();
	}
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

void AMyCharacter::CheckBackwardJumpEligibility()
{
	// Decides if you can jump backward
	if (FloorAngle < 0.45f)
		bHasBackwardJumpAngle = true;
	else
		bHasBackwardJumpAngle = false;

	// Check vector differences 
	if (const float VectorDifference = (GetActorForwardVector() - CharacterMovement).Length() > 1.7f)
		bIsTurningBackward = true;
	else
		bIsTurningBackward = false;
	
	if (FloorAngle < 0.77f && GetCharacterMovement()->Velocity.Z < 0.f && !bIsExhausted && CurrentState != Eps_Climbing)
	{
		GetCharacterMovement()->GravityScale = 0.f;
		GetCharacterMovement()->Velocity = FVector(0.f, 0.f, -250.f);
		GetCharacterMovement()->JumpZVelocity = 0;
	}

	else
	{
		GetCharacterMovement()->GravityScale = 2.f;
		GetCharacterMovement()->JumpZVelocity = 763;
	}
}

void AMyCharacter::CheckWallClimb()
{
	if (CantClimbTimer < 1.f)
	{
		CantClimbTimer += GetWorld()->DeltaTimeSeconds;
		return;
	}

	FVector StartWallAngle = GetActorLocation();
	FVector EndForwardAngle = GetActorLocation() + GetActorForwardVector() * 80.f;
	FVector EndRightAngle = GetActorLocation() + GetActorForwardVector() * 80.f + GetActorRightVector() * 25.f;
	FVector EndLeftAngle = GetActorLocation() + GetActorForwardVector() * 80.f - GetActorRightVector() * 25.f;

	FHitResult HitResultForward;
	FHitResult HitResultRight;
	FHitResult HitResultLeft;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);
	
	DrawDebugLine(GetWorld(), StartWallAngle, EndForwardAngle, FColor::Red, false, 0.5f, 0.f, 2.f);
	DrawDebugLine(GetWorld(), StartWallAngle, EndRightAngle, FColor::Green, false, 0.5f, 0.f, 2.f);
	DrawDebugLine(GetWorld(), StartWallAngle, EndLeftAngle, FColor::Blue, false, 0.5f, 0.f, 2.f);

	
	// Check if all LineTraces find the climbing wall. 
	if (GetWorld()->LineTraceSingleByChannel(HitResultForward, StartWallAngle, EndForwardAngle, ECC_GameTraceChannel2, Parameters, FCollisionResponseParams())
		&& GetWorld()->LineTraceSingleByChannel(HitResultRight, StartWallAngle, EndRightAngle, ECC_GameTraceChannel2, Parameters, FCollisionResponseParams())
		&& GetWorld()->LineTraceSingleByChannel(HitResultLeft, StartWallAngle, EndLeftAngle, ECC_GameTraceChannel2, Parameters, FCollisionResponseParams()))
	{
		GetCharacterMovement()->BrakingDecelerationFlying = FLT_MAX;
		CurrentState = Eps_Climbing;
		FRotator WallAngle = HitResultForward.ImpactNormal.Rotation();
		UE_LOG(LogTemp, Warning, TEXT("Wall angle: %s"), *WallAngle.ToString());
		SetActorRotation(FMath::Lerp(
			GetActorRotation(),
			FRotator(0.f, 180.f, 0.f)
			+ FRotator(-WallAngle.Pitch, WallAngle.Yaw, 0.f), /*WallAngle,*/
			0.05f));
	}

	else if (CurrentState == Eps_Climbing)
		StopClimbing();
}

void AMyCharacter::StopClimbing()
{
	if (CurrentState != Eps_Climbing)
		return;
	
	CantClimbTimer = 0.f;
	CurrentState = Eps_Walking;
	GetCharacterMovement()->MovementMode = MOVE_Walking;
}

void AMyCharacter::HandleMouseInputX(const float Value)
{
	MouseMovementX = Value * GetWorld()->DeltaTimeSeconds;
}

void AMyCharacter::HandleMouseInputY(const float Value)
{
	MouseMovementY = Value * GetWorld()->DeltaTimeSeconds;
}

void AMyCharacter::CameraMovementOutput()
{
	if (Controller == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter Line 125, no controller"));
		return;
	}
	
	CameraMovement = FVector2D(MouseMovementX, MouseMovementY);
	
	AddControllerYawInput(CameraMovement.X * CurrentCameraSpeed * GetWorld()->DeltaTimeSeconds);
	AddControllerPitchInput(-CameraMovement.Y * CurrentCameraSpeed * GetWorld()->DeltaTimeSeconds);
}

void AMyCharacter::CheckIdleness()
{
	TimeSinceMoved += GetWorld()->DeltaTimeSeconds;

	if (GetCharacterMovement()->Velocity.Length() != 0.f || CameraMovement.Length() != 0.f)
		TimeSinceMoved = 0.f;
	
	// Time in seconds before perceived as idle 
	if (TimeSinceMoved > 5.f && CurrentState != Eps_Idle)
	{
		SavedState = CurrentState;
		CurrentState = Eps_Idle;
	}
}

void AMyCharacter::HandleAimInput()
{
	if (CurrentState == Eps_Idle)
		return;

	// Save current state for later
	if (CurrentState != Eps_LeaveAiming)
		SavedState = CurrentState;
	
	CurrentState = Eps_Aiming;
}

void AMyCharacter::HandleAimStop()
{
	if (CurrentState == Eps_Idle)
	return;

	CurrentState = Eps_LeaveAiming;
}

void AMyCharacter::LookForHook()
{
	if (CurrentState != Eps_Aiming)
		return;
	
	FHitResult HookshotTarget;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);

	const FVector StartHookSearch = FollowCamera->GetComponentLocation(); /*GetActorLocation() + GetActorUpVector() * 50.f;*/
	const FVector EndHookSearch = FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector() * HookLength;

	DrawDebugLine(GetWorld(), StartHookSearch, EndHookSearch, FColor::Purple, false, 0.1f, 0.f, 3.f);

	if (GetWorld()->LineTraceSingleByChannel(HookshotTarget, StartHookSearch, EndHookSearch, ECollisionChannel::ECC_GameTraceChannel1, Parameters, FCollisionResponseParams()))
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		
		TargetLocation = HookshotTarget.Location - FollowCamera->GetForwardVector() * 50.f - FVector(0, 0, 100.f);
		bIsUsingHookShot = true;
		CurrentState = Eps_LeaveAiming;
		
		MoveToLocation(LatentInfo, HookshotTarget.Distance/1000.f);
	}
}

void AMyCharacter::MoveToLocation(const FLatentActionInfo& CurrentLatentInfo, const float Duration) const
{
	UKismetSystemLibrary::MoveComponentTo(
			RootComponent,
			TargetLocation,
			FRotator(0, FollowCamera->GetForwardVector().Rotation().Yaw, 0),
			false,
			false,
			Duration,
			false,
			EMoveComponentAction::Move,
			CurrentLatentInfo
			);
}

void AMyCharacter::SetSpringArmLength(const float NewLength, const float Alpha) const
{
	SpringArm->TargetArmLength = FMath::Lerp(
	SpringArm->TargetArmLength,
	NewLength,
	Alpha 
	);
}

void AMyCharacter::SetSpringArmOffset(const FVector& NewOffset, const float Alpha) const
{
	SpringArm->SocketOffset = FMath::Lerp(
	SpringArm->SocketOffset,
	NewOffset,
	Alpha
	);
}