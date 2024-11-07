#pragma once

struct EnHippieUnrealLibrary
{
public:
	static void MoveToLocation(AActor* ActorToMove, FVector EndLocation, FRotator EndRotation, float DistanceMargin, float MoveSpeed, float DeltaTime);

private:
	bool GetMovementPermission() { return bIsAllowedToMove; }
	static void SetMovementPermission(bool Value);
	inline static bool bIsAllowedToMove = false;

	static void MoveActor();
	// FVector MoveActor(/*AActor* ActorToMove, FVector EndLocation, FRotator EndRotation, float MoveSpeed, float DeltaTime*/);
	static FTimerHandle TimerMove;
};
