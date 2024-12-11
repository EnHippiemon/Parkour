// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyMovementModeComponent.generated.h"

// enum ECurrentMovementMode
// {
// 	Ecmm_Idle,
// 	Ecmm_Walking,
// 	Ecmm_Sprinting,
// 	Ecmm_Climbing,
// 	Ecmm_LedgeClimbing,
// 	Ecmm_Jumping,
// 	Ecmm_ClimbJumping,
// 	Ecmm_RunningUpWall,
// 	Ecmm_WallJumping,
// 	Ecmm_Aiming,
// 	Ecmm_LeavingAim,
// 	Ecmm_Exhausted
// };

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARKOUR_API UMyMovementModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMyMovementModeComponent();

private:
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
