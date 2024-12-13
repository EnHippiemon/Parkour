#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/ClimbMovementDataAsset.h"
#include "MyClimbComponent.generated.h"

class AMyCharacter;

class UClimbMovementDataAsset;

enum EPlayerState;

UCLASS( Blueprintable, BlueprintType, ClassGroup=(My), meta=(BlueprintSpawnableComponent) )
class PARKOUR_API UMyClimbComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMyClimbComponent();
	
	EPlayerState FindClimbableWall();

	bool CheckCanClimb() { return CantClimbTimer >= ClimbData->ClimbJumpingTime; }
	
	void ResetCantClimbTimer() { CantClimbTimer = 0; }
	void SetIsJumpingOutFromWall(bool Value) { bIsJumpingOutFromWall = Value; }
	bool IsClimbingLedge() { return bIsClimbingLedge; }

	EPlayerState StopClimbing();
	
private:

	/* Climb jump */// Impulse or velocity? 
	float CantClimbTimer = 0.f;
	bool bIsJumpingOutFromWall;
	UPROPERTY()
	AActor* CurrentClimbingWall;

	/* Ledge climbing */
	FVector LedgeClimbDestination; 
	bool bIsClimbingLedge = false;
	
	UPROPERTY()
	AMyCharacter* Player;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
	UClimbMovementDataAsset* ClimbData;
	
	/* Climbing */
	void FindClimbRotation();
	void SetPlayerRotation(const FRotator& TargetRotation);

	void ForcePlayerOntoWall();

	void LookForLedge();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
