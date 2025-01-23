#include "PlayerHUDWidget.h"

#include "Characters/MyCharacter.h"
#include "Characters/MyMovementModeComponent.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (IsValid(Player))
	{
		Player->OnAim.AddUniqueDynamic(this, &UPlayerHUDWidget::ActivateCrosshair);
		Player->OnStopAim.AddUniqueDynamic(this, &UPlayerHUDWidget::DeactivateCrosshair);
		
		if (IsValid(Player->GetMovementModeComponent()))
			Player->GetMovementModeComponent()->OnNewMovement.AddUniqueDynamic(this, &UPlayerHUDWidget::UpdateMovementImage);
	}

	CrosshairImage->SetOpacity(0.f);
}

float UPlayerHUDWidget::CalculateEnergyPercentage()
{
	if (!Player)
		return 0.f;

	return Player->GetMovementEnergy();
}

void UPlayerHUDWidget::UpdateMovementImage()
{
	MovementImage->SetBrushFromTexture(Player->GetMovementModeComponent()->GetCurrentMovementTexture(), true);
}

void UPlayerHUDWidget::ActivateCrosshair()
{
	CrosshairImage->SetOpacity(1.f);
}

void UPlayerHUDWidget::DeactivateCrosshair()
{
	CrosshairImage->SetOpacity(0.f);
}