#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class AMyCharacter;

UCLASS()
class PARKOUR_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintPure)
	float CalculateEnergyPercentage();

	UPROPERTY(VisibleAnywhere)
	AMyCharacter* Player;
};
