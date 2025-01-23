#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UImage;
class FCanvasItem;
class AMyCharacter;

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
	UFUNCTION()
	void ActivateCrosshair();
	UFUNCTION()
	void DeactivateCrosshair();

	UPROPERTY(VisibleAnywhere)
	AMyCharacter* Player;

	UPROPERTY(meta = (BindWidget), EditDefaultsOnly)
	UImage* MovementImage;

	UPROPERTY(meta = (BindWidget), EditDefaultsOnly)
	UImage* CrosshairImage;

	UPROPERTY(EditDefaultsOnly)
	FVector2D ImageOffset = FVector2D(-10, 0);
};