#include "PlayerHUDWidget.h"

#include "MyCharacter.h"
#include "Kismet/GameplayStatics.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

float UPlayerHUDWidget::CalculateEnergyPercentage()
{
	if (!Player)
		return 0.f;

	return Player->MovementEnergy;
}
