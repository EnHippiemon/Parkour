#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/ClimbMovementDataAsset.h"
#include "MyClimbComponent.generated.h"

class AMyCharacter;

class UEnergyDataAsset;
class UClimbMovementDataAsset;

enum EPlayerState;

UCLASS( Blueprintable, BlueprintType, ClassGroup=(My), meta=(BlueprintSpawnableComponent) )
class PARKOUR_API UMyClimbComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMyClimbComponent();
	
	EPlayerState FindClimbableWall();

	bool CheckCanClimb() const { return CantClimbTimer >= ClimbData->ClimbJumpingTime; }
	
	void ResetCantClimbTimer() { CantClimbTimer = 0; }
	void SetIsJumpingOutFromWall(const bool Value) { bIsJumpingOutFromWall = Value; }
	bool GetIsJumpingOutFromWall() const { return bIsJumpingOutFromWall; }
	bool GetIsClimbingLedge() const { return bIsClimbingLedge; }

	EPlayerState StopClimbing();

	void BoostEnergy();
	
private:
	/* Climb jump */ 
		float CantClimbTimer = 0.f;
		bool bIsJumpingOutFromWall;
		UPROPERTY()
		AActor* CurrentClimbingWall;
		double WallPitchRotation;

	/* Ledge climbing */
		FVector LedgeClimbDestination; 
		bool bIsClimbingLedge = false;
		
		UPROPERTY()
		AMyCharacter* Player;
		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
		UClimbMovementDataAsset* ClimbData;
	
	/* Climbing */
		void FindClimbRotation();
		void SetPlayerRotation(const FRotator& TargetRotation) const;

		void ForcePlayerOntoWall() const;

		void LookForLedge();

		UFUNCTION()
		void StateSwitch(EPlayerState NewState);

		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
		UEnergyDataAsset* EnergyData;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};