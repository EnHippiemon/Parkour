#include "PlayerHUDWidget.h"

#include "Characters/MyCharacter.h"
#include "Characters/MyMovementModeComponent.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// if (IsValid(Player))
	// {
	// 	if (IsValid(Player->GetMovementModeComponent()))
	// 		Player->GetMovementModeComponent()->OnNewMovement.AddUniqueDynamic(this, &UPlayerHUDWidget::UpdateMovementImage);
	// }
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