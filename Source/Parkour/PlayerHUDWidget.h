#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UImage;
class FCanvasItem;
class AMyCharacter;
// class UCanvas;

UCLASS()
class PARKOUR_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintPure)
	float CalculateEnergyPercentage();

	UFUNCTION()
	void UpdateMovementImage();

	UPROPERTY(VisibleAnywhere)
	AMyCharacter* Player;

	UPROPERTY(meta = (BindWidget), EditDefaultsOnly)
	UImage* MovementImage;

	UPROPERTY(EditDefaultsOnly)
	FVector2D ImageOffset = FVector2D(-10, 0);

	
};
