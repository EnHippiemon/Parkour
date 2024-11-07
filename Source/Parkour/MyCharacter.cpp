#include "MyCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnHippieUnrealLibrary.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 50.f;
	SpringArm->CameraLagSpeed = 20.f;
	SpringArm->CameraLagMaxDistance = 3.f;

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


void AMyCharacter::PlayerStateSwitch()
{
	switch (CurrentState)
	{
	case Eps_Walking:
		GetCharacterMovement()->MaxWalkSpeed = 600.f * MovementSpeedPercent * MovementEnergy;
		GetCharacterMovement()->SetWalkableFloorAngle(50.f);
		SpringArm->CameraLagSpeed = 20.f;
		SpringArm->CameraLagMaxDistance = 3.f;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		CheckFloorAngle();
		CheckIdleness();
		// SpringArm->TargetArmLength = FMath::Lerp(SpringArm->TargetArmLength, StandardSpringArmLength, NormalCameraSwitchSpeed);
		// What is below is probably useless. Just use the above?
		MyLerp(SpringArm->TargetArmLength, StandardSpringArmLength, NormalCameraSwitchSpeed);
		if (GetCharacterMovement()->Velocity.Length() != 0)
			MyLerp(FollowCamera->FieldOfView, WalkingFOV, NormalCameraSwitchSpeed/4);
		else
			MyLerp(FollowCamera->FieldOfView, StillFOV, NormalCameraSwitchSpeed);
		break;

	case Eps_Sprinting:
		GetCharacterMovement()->MaxWalkSpeed = 1000.f * MovementSpeedPercent * MovementEnergy;
		GetCharacterMovement()->SetWalkableFloorAngle(90.f);
		CheckFloorAngle();
		MyLerp(SpringArm->TargetArmLength, SprintingSpringArmLength, SpringArmSwitchSpeed);
		MyLerp(FollowCamera->FieldOfView, SprintingFOV, SprintFOVSpeed);
		if (GetCharacterMovement()->Velocity.Length() < 0.1f)
			HandleSprintStop();
		break;
		
	case Eps_Aiming:
		CurrentCameraSpeed = AimCameraSpeed;
		SpringArm->CameraRotationLagSpeed = 1000.f;
		MovementEnergy -= GetWorld()->DeltaTimeSeconds * AimEnergyDepletionSpeed;
		MyLerp(SpringArm->TargetArmLength, 100.f, 0.3f);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, AimRotationRate, 0.f);
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.05f);
		MyLerp(SpringArm->SocketOffset, AimingCameraOffset, AimingCameraTransitionAlpha);
		MyLerp(FollowCamera->FieldOfView, AimingFOV, AimingCameraTransitionAlpha);
		break;

	case Eps_LeaveAiming:
		SpringArm->CameraRotationLagSpeed = 50.f;
		CurrentCameraSpeed = StandardCameraSpeed;
		GetCharacterMovement()->RotationRate = FRotator(0.f, StandardRotationRate, 0.f);
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
		MyLerp(SpringArm->TargetArmLength, StopAimingSpringArmLength, NormalCameraSwitchSpeed);
		MyLerp(FollowCamera->FieldOfView, WalkingFOV, NormalCameraSwitchSpeed);
		
		if (bIsUsingHookshot)
			GetCharacterMovement()->Velocity = FVector(0, 0, 0);

		if (bIsUsingHookshot && (GetActorLocation() - TargetLocation).Length() < 10.f)
			bIsUsingHookshot = false;

		if (!bIsUsingHookshot && SavedState == Eps_Sprinting)
			CurrentState = Eps_Walking;
		
		else if (!bIsUsingHookshot)
			CurrentState = SavedState;
		break;

	case Eps_Idle:
		CheckIdleness();
		AddControllerYawInput(GetWorld()->DeltaTimeSeconds * 2);
		MyLerp(SpringArm->TargetArmLength, 10000.f, 0.0001f);
		MyLerp(FollowCamera->FieldOfView, 60.f, 0.0001f);
		
		if (SpringArm->GetTargetRotation().Vector().Z > -0.25f)
			AddControllerPitchInput(GetWorld()->DeltaTimeSeconds / 2);
			
		else if (SpringArm->GetTargetRotation().Vector().Z < -0.3f)
			AddControllerPitchInput(-GetWorld()->DeltaTimeSeconds / 2);
		
		if (TimeSinceMoved < 2.f)
			CurrentState = SavedState;
		break;

	case Eps_Climbing:
		GetCharacterMovement()->MovementMode = MOVE_Flying;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		MyLerp(SpringArm->TargetArmLength, StandardSpringArmLength, NormalCameraSwitchSpeed);
		MyLerp(FollowCamera->FieldOfView, WalkingFOV, NormalCameraSwitchSpeed);
		SpringArm->CameraLagSpeed = 1.f;
		SpringArm->CameraLagMaxDistance = 10.f;

		// Makes sure that the player is pushed to the wall and doesn't fall off
		if (CantClimbTimer >= 1.f)
			GetCharacterMovement()->AddImpulse(GetActorForwardVector() * 1000.f);
		break;
		
	default:
		UE_LOG(LogTemp, Error, TEXT("No active Player State. Now Walking"))
		CurrentState = Eps_Walking;
		break;
	}

	if (CurrentState != Eps_Climbing)
		MyLerp(CurrentCameraOffsetZ, 0.f, NormalCameraSwitchSpeed);
	
	if (CurrentState != Eps_Aiming)
		MyLerp(SpringArm->SocketOffset, FVector(
			0.f, CurrentCameraOffsetY, CurrentCameraOffsetZ), NormalCameraSwitchSpeed);

	if (CurrentState != Eps_Idle)
		MyLerp(FollowCamera->FieldOfView, WalkingFOV, 0.001f);
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

	// If the line trace is null distance = 0. Then setting distance to max so the other ones are shorter. 
	if (!GetWorld()->LineTraceSingleByChannel(HitResultForward, StartFloorAngle, EndForwardAngle, ECC_GameTraceChannel18, Parameters, FCollisionResponseParams()))
		HitResultForward.Distance = FLT_MAX;
	
	if (!GetWorld()->LineTraceSingleByChannel(HitResultRight, StartFloorAngle, EndRightAngle, ECC_GameTraceChannel18, Parameters, FCollisionResponseParams()))
		HitResultRight.Distance = FLT_MAX;
	
	if (!GetWorld()->LineTraceSingleByChannel(HitResultLeft, StartFloorAngle, EndLeftAngle, ECC_GameTraceChannel18, Parameters, FCollisionResponseParams()))
		HitResultLeft.Distance = FLT_MAX;

	FloorAngle = FMath::Min3(HitResultForward.Distance, HitResultLeft.Distance, HitResultRight.Distance) / 100.f;
	
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

void AMyCharacter::CheckExhaustion()
{
	float MovementLoss = 0.f;
	
	if (MovementEnergy >= 1.f)
		bIsExhausted = false;
	
	if (MovementEnergy <= 0.f && !bIsExhausted)
	{
		bIsExhausted = true;
		if (CurrentState == Eps_Aiming)
			CurrentState = Eps_LeaveAiming;
		else 
			CurrentState = Eps_Walking;
	}

	// I just wanted some exponential energy drain depending on the floor angle. Above is better though
	// else if (FloorAngle < 0.8f
	// 	&& MovementEnergy > 0.f
	// 	&& GetCharacterMovement()->Velocity.Length() > 0.f
	// 	&& CurrentState == Eps_Sprinting)
	// 	MovementEnergy -= GetWorld()->DeltaTimeSeconds * ExhaustionSpeed / FMath::Pow(10, FloorAngle) * 3;
	//
	
	// else if (FloorAngle < 0.74f
	// 	&& MovementEnergy > 0.f
	// 	&& CurrentState == Eps_Sprinting)
	// {
	// 	MovementEnergy -= GetWorld()->DeltaTimeSeconds * 
	// 	MovementLoss = GetWorld()->DeltaTimeSeconds * ExhaustionSpeed;
	// }
	
	else if (FloorAngle < 0.74f
		&& MovementEnergy > 0.f
		&& GetCharacterMovement()->Velocity.Length() > 0.f
		&& CurrentState == Eps_Sprinting)
		MovementLoss = GetWorld()->DeltaTimeSeconds * ExhaustionSpeed;
		
	else if (FloorAngle < 0.77f
		&& MovementEnergy > 0.f
		&& GetCharacterMovement()->Velocity.Length() > 0.f
		&& CurrentState == Eps_Sprinting)
		MovementLoss = GetWorld()->DeltaTimeSeconds * ExhaustionSpeed / 2;
	
	else if (FloorAngle < 0.8f
		&& MovementEnergy > 0.f
		&& GetCharacterMovement()->Velocity.Length() > 0.f
		&& CurrentState == Eps_Sprinting)
		MovementLoss = GetWorld()->DeltaTimeSeconds * ExhaustionSpeed / 4;
	

	else if (MovementEnergy < 1.f)
		MovementEnergy += GetWorld()->DeltaTimeSeconds * ExhaustionSpeed;
	
	// else if (FloorAngle < 0.74f
	// 	&& MovementEnergy > 0.f
	// 	&& GetCharacterMovement()->Velocity.Length() > 0.f
	// 	&& CurrentState == Eps_Sprinting)
	// 	MovementEnergy -= GetWorld()->DeltaTimeSeconds * ExhaustionSpeed;
	// 	
	// else if (FloorAngle < 0.77f
	// 	&& MovementEnergy > 0.f
	// 	&& GetCharacterMovement()->Velocity.Length() > 0.f
	// 	&& CurrentState == Eps_Sprinting)
	// 	MovementEnergy -= GetWorld()->DeltaTimeSeconds * ExhaustionSpeed / 2;
	//
	// else if (FloorAngle < 0.8f
	// 	&& MovementEnergy > 0.f
	// 	&& GetCharacterMovement()->Velocity.Length() > 0.f
	// 	&& CurrentState == Eps_Sprinting)
	// 	MovementEnergy -= GetWorld()->DeltaTimeSeconds * ExhaustionSpeed / 4;
	//
	// // I just wanted some exponential energy drain depending on the floor angle. Above is better though
	// // else if (FloorAngle < 0.8f
	// // 	&& MovementEnergy > 0.f
	// // 	&& GetCharacterMovement()->Velocity.Length() > 0.f
	// // 	&& CurrentState == Eps_Sprinting)
	// // 	MovementEnergy -= GetWorld()->DeltaTimeSeconds * ExhaustionSpeed / FMath::Pow(10, FloorAngle) * 3;
	// //
	// else if (MovementEnergy < 1.f)
	// 	MovementEnergy += GetWorld()->DeltaTimeSeconds * ExhaustionSpeed;
	
	// Decide movement speed
	if (FloorAngle == 0.f)
		FloorAngle = 1.f;

	else if (FloorAngle < 0.8f)
		FloorAngle = FloorAngle * FloorAngle * FloorAngle;
	
	else if (FloorAngle < 1.f)
		FloorAngle *= FloorAngle;
	
	MovementSpeedPercent = FloorAngle;
	// UE_LOG(LogTemp, Warning, TEXT("MovementLoss: %f"), MovementLoss);

	MovementEnergy -= MovementLoss;
}

void AMyCharacter::HandleJumpInput()
{
	if (CurrentState == Eps_Climbing && MovementEnergy > 0.f && CantClimbTimer >= 1.f)
	{
		CantClimbTimer = 0.5f;
		MovementEnergy -= 0.3f;
		GetCharacterMovement()->BrakingDecelerationFlying = 1000.f;
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

	// Check if all LineTraces find the climbing wall. 
	if (GetWorld()->LineTraceSingleByChannel(HitResultForward, StartWallAngle, EndForwardAngle, ECC_GameTraceChannel2, Parameters, FCollisionResponseParams())
		&& GetWorld()->LineTraceSingleByChannel(HitResultRight, StartWallAngle, EndRightAngle, ECC_GameTraceChannel2, Parameters, FCollisionResponseParams())
		&& GetWorld()->LineTraceSingleByChannel(HitResultLeft, StartWallAngle, EndLeftAngle, ECC_GameTraceChannel2, Parameters, FCollisionResponseParams()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Found wall to climb"));
		
		if (CurrentState == Eps_Aiming)
		{
			SavedState = Eps_Climbing;
			CurrentState = Eps_LeaveAiming;
		}

		else
		{
			GetCharacterMovement()->BrakingDecelerationFlying = FLT_MAX;
			CurrentState = Eps_Climbing;
			FRotator WallAngle = HitResultForward.ImpactNormal.Rotation();
			SetActorRotation(FMath::Lerp(
				GetActorRotation(),
				FRotator(0.f, 180.f, 0.f)
				+ FRotator(-WallAngle.Pitch, WallAngle.Yaw, 0.f), /*WallAngle,*/
				0.05f));
		}
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
	CurrentState == Eps_Climbing ? CameraSpeed = CameraYDirectionSpeed * 2
	: CameraSpeed = CameraYDirectionSpeed / (CharacterMovement.Length() + 1);
	
	if (CameraMovement.X < -0.005f)
			SetCurrentOffset(CurrentCameraOffsetY, -CameraSpeed, CameraClamp.Y);

	else if (CameraMovement.X > 0.005f)
			SetCurrentOffset(CurrentCameraOffsetY, CameraSpeed, CameraClamp.Y);

	if (CurrentState == Eps_Climbing)
	{
		if (CameraMovement.Y < -0.005f)
				SetCurrentOffset(CurrentCameraOffsetZ, -CameraSpeed, CameraClamp.Z);

		else if (CameraMovement.Y > 0.005f)
				SetCurrentOffset(CurrentCameraOffsetZ, CameraSpeed, CameraClamp.Z);
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

void AMyCharacter::HandleAimInput()
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

void AMyCharacter::HandleAimStop()
{
	if (CurrentState == Eps_Idle)
	return;

	CurrentState = Eps_LeaveAiming;
}

void AMyCharacter::SetCurrentOffset(float& Value, const float Speed, const float Clamp) const
{
	Value += Speed * GetWorld()->DeltaTimeSeconds;
	Value = FMath::Clamp(Value, -Clamp, Clamp);
}

void AMyCharacter::LookForHook()
{
	if (CurrentState != Eps_Aiming)
		return;
	
	FHitResult HookshotTarget;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(this);

	const FVector StartHookSearch = FollowCamera->GetComponentLocation();
	const FVector EndHookSearch = FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector() * HookLength;

	DrawDebugLine(GetWorld(), StartHookSearch, EndHookSearch, FColor::Purple, false, 0.1f, 0.f, 3.f);

	if (GetWorld()->LineTraceSingleByChannel(HookshotTarget, StartHookSearch, EndHookSearch, ECollisionChannel::ECC_GameTraceChannel1, Parameters, FCollisionResponseParams()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Found hook"));
		
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		
		TargetLocation = HookshotTarget.Location - FollowCamera->GetForwardVector() * 50.f - FVector(0, 0, 100.f);
		bIsUsingHookshot = true;
		CurrentState = Eps_LeaveAiming;
		
		// EnHippieUnrealLibrary::MoveToLocation(
		// 	this,
		// 	TargetLocation,
		// 	FRotator(0, FollowCamera->GetForwardVector().Rotation().Yaw, 0),
		// 	5,
		// 	HookshotTarget.Distance/HookshotSpeed,
		// 	GetWorld()->DeltaTimeSeconds);
		//
		MoveToLocation(LatentInfo, HookshotTarget.Distance/HookshotSpeed);
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

template <typename T1, typename T2>
void AMyCharacter::MyLerp(T1& A, T2 B, const float Alpha)
{
	A = FMath::Lerp(A, B, Alpha);
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
	CheckWallClimb();

	// Keep last 
	PlayerStateSwitch();

	// UE_LOG(LogTemp, Warning, TEXT("Player velocity: %s"), *GetCharacterMovement()->Velocity.ToString())
	// UE_LOG(LogTemp, Warning, TEXT("Saved state: %d"), SavedState.GetValue())
	// UE_LOG(LogTemp, Warning, TEXT("Player state: %d"), CurrentState.GetValue())
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
