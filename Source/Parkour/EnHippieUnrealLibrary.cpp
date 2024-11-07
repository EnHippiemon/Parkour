#include "EnHippieUnrealLibrary.h"

void EnHippieUnrealLibrary::MoveToLocation(AActor* ActorToMove, FVector EndLocation, FRotator EndRotation, float DistanceMargin, float MoveSpeed, float DeltaTime)
{
	// Check if player is close to target
	if ((EndLocation - ActorToMove->GetActorLocation()).Length() < DistanceMargin)
	{
		// Do the thing
		return;
	}

	const auto CurrentLocation = FMath::Lerp(
		ActorToMove->GetActorLocation(),
		EndLocation,
		MoveSpeed * DeltaTime
		);
	ActorToMove->SetActorLocation(CurrentLocation);

	const auto CurrentRotation = FMath::Lerp(
		ActorToMove->GetActorRotation(),
		EndRotation,
		MoveSpeed * DeltaTime
		);
	ActorToMove->SetActorRotation(CurrentRotation);
    
	MoveToLocation(ActorToMove, EndLocation, EndRotation, DistanceMargin, MoveSpeed, DeltaTime);
	
	/*
	if (!IsValid(ActorToMove))
		return;

	SetMovementPermission(true);
	// bool ReachedEnd = false
	// if ReachedEnd == false then loop else stop 
	// auto TickTime =
	
	// ActorToMove->GetWorldTimerManager().SetTimer(TimerMove, , EnHippieUnrealLibrary::MoveActor, DeltaTime, true);

	int i = 0;
	while (bIsAllowedToMove)
	{
		i++;
		UE_LOG(LogTemp, Warning, TEXT("Moving: %i"), i);
		const auto CurrentLocation = FMath::Lerp(
			ActorToMove->GetActorLocation(),
			EndLocation,
			MoveSpeed * DeltaTime
			);
		ActorToMove->SetActorLocation(CurrentLocation);

		const auto CurrentRotation = FMath::Lerp(
			ActorToMove->GetActorRotation(),
			EndRotation,
			MoveSpeed * DeltaTime
			);
		ActorToMove->SetActorRotation(CurrentRotation);

		if ((ActorToMove->GetActorLocation() - EndLocation).Length() < DistanceMargin)
		{
			SetMovementPermission(false);
		}
	}
	*/
}

void EnHippieUnrealLibrary::SetMovementPermission(bool Value)
{
	bIsAllowedToMove = Value;
}

void EnHippieUnrealLibrary::MoveActor()
{
	
}
