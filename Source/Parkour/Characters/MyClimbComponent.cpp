#include "../Characters/MyClimbComponent.h"

#include "MyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DataAssets/ClimbMovementDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UMyClimbComponent::UMyClimbComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	static ConstructorHelpers::FObjectFinder<UClimbMovementDataAsset> ClimbMovement(TEXT("/Game/Player/DataAssets/ClimbMovementDataAsset"));
	if (ClimbMovement.Object)
		ClimbData = ClimbMovement.Object;
}

EPlayerState UMyClimbComponent::FindClimbableWall()
{
	if (bIsClimbingLedge || Player->GetIsUsingHookshot())
		return Player->GetCurrentState();
	
	if (CantClimbTimer < ClimbData->ClimbJumpingTime)
	{
		CantClimbTimer += GetWorld()->DeltaTimeSeconds;
		FindClimbRotation();
		return Player->GetCurrentState();
	}

	const auto World = GetWorld();
	const auto ActorLocation = Player->GetActorLocation();
	const auto ForwardVector = Player->GetActorForwardVector();
	const auto RightVector = Player->GetActorRightVector();

	constexpr int TraceLength = 80;
	const FVector StartWallAngle = ActorLocation;
	const FVector EndForwardAngle = ActorLocation + ForwardVector * TraceLength;
	const FVector EndRightAngle = ActorLocation + ForwardVector * TraceLength + RightVector * ClimbData->ClimbingWidth;
	const FVector EndLeftAngle = ActorLocation + ForwardVector * TraceLength - RightVector * ClimbData->ClimbingWidth;

	FHitResult HitResultForward;
	FHitResult HitResultRight;
	FHitResult HitResultLeft;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(Player);

	const auto ForwardTrace = World->LineTraceSingleByChannel(HitResultForward, StartWallAngle, EndForwardAngle, ClimbData->ClimbingCollision, Parameters, FCollisionResponseParams());
	const auto RightTrace = World->LineTraceSingleByChannel(HitResultRight, StartWallAngle, EndRightAngle, ClimbData->ClimbingCollision, Parameters, FCollisionResponseParams());
	const auto LeftTrace = World->LineTraceSingleByChannel(HitResultLeft, StartWallAngle, EndLeftAngle, ClimbData->ClimbingCollision, Parameters, FCollisionResponseParams());

	// Check if two LineTraces find a climbing wall. 
	if (ForwardTrace && RightTrace ||
		ForwardTrace && LeftTrace ||
		RightTrace && LeftTrace)
	{
		if (Player->GetCurrentState() == Eps_Aiming)
		{
			Player->SetSavedState(Eps_Climbing);
			return Eps_LeaveAiming;
		}
		// GetCharacterMovement()->BrakingDecelerationFlying = FLT_MAX;
		Player->SetNewAnimation(Ecmm_Climbing);
		Player->SetDeceleration(FLT_MAX);
		FindClimbRotation();
		return Eps_Climbing;
	}
	
	return StopClimbing();
}

void UMyClimbComponent::FindClimbRotation()
{
	FHitResult HitResultPlayerRotation;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(Player);

	// Cache the getters, because all of them are used several times. 
	const auto World = GetWorld();
	const auto ActorLocation = Player->GetActorLocation();
	const auto ForwardVector = Player->GetActorForwardVector();
	const auto RightVector = Player->GetActorRightVector();
	const auto UpVector = Player->GetActorUpVector();
	const auto CapsuleRadius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius() * 0.5f;
	const auto MovementSideways = Player->GetMovementSideways();
	const auto MovementForward = Player->GetMovementForward();

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
	const FVector EndFrontWallDetection = ActorLocation + UpVector * MovementForward * ClimbData->PlayerToWallDistance + RightVector *
										  MovementSideways * ClimbData->PlayerToWallDistance + ForwardVector * 80.f;
	EndTraces.Add(EndFrontWallDetection);
	
	const FVector StartEyesightWallDetection = ActorLocation;
	StartTraces.Add(StartEyesightWallDetection);
	const FVector EndEyesightWallDetection = ActorLocation + ForwardVector * 100.f;
	EndTraces.Add(EndEyesightWallDetection);
#pragma endregion
	
	bool WallRotationTrace = false;
	for (int i = 0; i < StartTraces.Num(); ++i)
	{
		WallRotationTrace = World->LineTraceSingleByChannel(HitResultPlayerRotation, StartTraces[i], EndTraces[i], ClimbData->ClimbingCollision, Parameters, FCollisionResponseParams());
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

void UMyClimbComponent::SetPlayerRotation(const FRotator& TargetRotation)
{
	Player->SetActorRotation(FMath::Lerp(
		Player->GetActorRotation(),
		FRotator(0.f, 180.f, 0.f)
		+ FRotator(-TargetRotation.Pitch, TargetRotation.Yaw, 0.f),
		0.05f));
}

// Interrupt climbing
EPlayerState UMyClimbComponent::StopClimbing()
{
	if (Player->GetCurrentState() != Eps_Climbing)
		return Player->GetCurrentState();
	
	CantClimbTimer = 0.f;
	Player->SetMovementMode(MOVE_Walking);
	return Eps_Walking;
}

void UMyClimbComponent::ForcePlayerOntoWall()
{
	if (Player->GetCurrentState() != Eps_Climbing)
		return;
	
	if (CantClimbTimer >= ClimbData->ClimbJumpingTime)
		Player->GetCharacterMovement()->AddImpulse(Player->GetActorForwardVector() * 1000.f);
}

void UMyClimbComponent::LookForLedge()
{
	if (bIsClimbingLedge)
	{
		if (FMath::IsNearlyZero((Player->GetActorLocation() - LedgeClimbDestination).Length()))
			bIsClimbingLedge = false;
		return;
	}
	if (!Player->GetIsMidAir() || Player->GetIsUsingHookshot())
		return; 
 
	auto World = GetWorld();
	const FVector LowTraceStart = Player->GetActorLocation() + Player->GetActorUpVector() * ClimbData->BottomLedgeDetectionZOffset;
	const FVector LowTraceEnd = LowTraceStart + Player->GetActorForwardVector() * ClimbData->LedgeClimbDetectionOffset.X;
	const FVector HighTraceStart = Player->GetActorLocation() + Player->GetActorUpVector() * ClimbData->LedgeClimbDetectionOffset.Z;
	const FVector HighTraceEnd = HighTraceStart + Player->GetActorForwardVector() * ClimbData->LedgeClimbDetectionOffset.X;
	
	FHitResult LowHitResult;
	FHitResult HighHitResult;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(Player);
	const auto LowTrace = World->LineTraceSingleByChannel(LowHitResult, LowTraceStart, LowTraceEnd, ClimbData->LedgeChannel, Parameters, FCollisionResponseParams());
	const auto HighTrace = World->LineTraceSingleByChannel(HighHitResult, HighTraceStart, HighTraceEnd, Player->GetBlockCollision(), Parameters, FCollisionResponseParams());

	// DrawDebugLine(World, LowTraceStart, LowTraceEnd, FColor::Red, false, EDrawDebugTrace::ForOneFrame);
	// DrawDebugLine(World, HighTraceStart, HighTraceEnd, FColor::Blue, false, EDrawDebugTrace::ForOneFrame);
	if (LowTrace && !HighTrace)
	{
		LedgeClimbDestination = HighTraceEnd + FVector(0, 0, Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		bIsClimbingLedge = true;
		Player->SetNewAnimation(Ecmm_LedgeClimbing);
		Player->MovePlayer(LedgeClimbDestination, ClimbData->LedgeClimbDuration);
	}
}

void UMyClimbComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(Player))
		Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}


void UMyClimbComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(Player))
	{
		UE_LOG(LogTemp, Error, TEXT("MyClimbComponent.cpp - no player found!"));
		return;
	}
	
	FindClimbableWall();
	ForcePlayerOntoWall();
	LookForLedge();
}