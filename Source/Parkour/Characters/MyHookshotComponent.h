#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyHookshotComponent.generated.h"

enum EPlayerState : int;

class AMyCharacter;
class UHookshotDataAsset;
class UGroundMovementDataAsset;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARKOUR_API UMyHookshotComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMyHookshotComponent();

	EPlayerState UseHookshot(AMyCharacter* ThisPlayer);
	EPlayerState LeaveAiming(const AMyCharacter* ThisPlayer);

	bool GetIsUsingHookshot() const { return bIsUsingHookshot; }
	float GetSlowMotionTimeDilation() const { return SlowMotionDilation; }

private:
	bool bIsUsingHookshot = false;
	FVector TargetLocation;
	
	UPROPERTY(EditDefaultsOnly, Category=TimeDilation)
	float SlowMotionDilation = 0.05f;


	void SetAnimation(const AMyCharacter* ThisPlayer) const;

	UFUNCTION()
	void StateSwitch(EPlayerState NewState);
	void SetTimeDilation();

	UPROPERTY()
	AMyCharacter* Player;

	/* Data assets */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
	UHookshotDataAsset* HookshotData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
	UGroundMovementDataAsset* GroundMovementData;
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
