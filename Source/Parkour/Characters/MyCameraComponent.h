#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Engine/DataAsset.h"
#include "MyCameraComponent.generated.h"

class AMyCharacter;
enum EPlayerState;

class UCameraDataAsset;

UCLASS(Blueprintable, ClassGroup=(My), meta=(BlueprintSpawnableComponent))
class PARKOUR_API UMyCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

private:
	UMyCameraComponent();
	
	UFUNCTION()
	void StateSwitch(EPlayerState State);
	void TickStateSwitch();

	EPlayerState CurrentState;

	UPROPERTY(VisibleAnywhere)
	AMyCharacter* Player;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
	UCameraDataAsset* CameraData;
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};