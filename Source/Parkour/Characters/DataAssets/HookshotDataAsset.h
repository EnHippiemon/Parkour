#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HookshotDataAsset.generated.h"

UCLASS()
class PARKOUR_API UHookshotDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/* Hookshot */ 
		// Higher is slower
		UPROPERTY(EditDefaultsOnly, Category=Hookshot)
		float HookshotSpeed = 1500.f;
		UPROPERTY(EditDefaultsOnly, Category=Hookshot)
		float HookLength = 1500.f;
		UPROPERTY(EditDefaultsOnly, Category=Hookshot)
		TEnumAsByte<ECollisionChannel> HookCollision;
};
