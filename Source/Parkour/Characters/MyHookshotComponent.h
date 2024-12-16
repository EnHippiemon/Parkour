#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyHookshotComponent.generated.h"

enum EPlayerState : int;

class AMyCharacter;
class UHookshotDataAsset;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARKOUR_API UMyHookshotComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMyHookshotComponent();

	EPlayerState UseHookshot(AMyCharacter* Player);
	EPlayerState LeaveAiming(AMyCharacter* Player);

	bool GetIsUsingHookshot() const { return bIsUsingHookshot; }
	
private:
	bool bIsUsingHookshot = false;
	FVector TargetLocation;

	void SetAnimation(AMyCharacter* Player);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
	UHookshotDataAsset* HookshotData;
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
