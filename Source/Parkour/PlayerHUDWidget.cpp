#include "PlayerHUDWidget.h"

#include "Characters/MyCharacter.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (Player)
		Player->OnNewMovement.AddUniqueDynamic(this, &UPlayerHUDWidget::UpdateMovementImage);
}

float UPlayerHUDWidget::CalculateEnergyPercentage()
{
	if (!Player)
		return 0.f;

	return Player->GetMovementEnergy();
}

void UPlayerHUDWidget::UpdateMovementImage()
{
	MovementImage->SetBrushFromTexture(Player->GetCurrentMovementTexture(), true);
}