#include "../Characters/MyHookshotComponent.h"

#include "MyCameraComponent.h"
#include "MyCharacter.h"
#include "DataAssets/HookshotDataAsset.h"

UMyHookshotComponent::UMyHookshotComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UHookshotDataAsset> Hookshot(TEXT("/Game/Player/DataAssets/HookshotDataAsset"));
	if (Hookshot.Object)
		HookshotData = Hookshot.Object;
}

EPlayerState UMyHookshotComponent::UseHookshot(AMyCharacter* Player)
{
	if (Player->GetCurrentState() != Eps_Aiming)
		return Player->GetCurrentState();

	FHitResult HookshotTarget;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(Player);

	const FVector StartHookSearch = Player->GetCameraComponent()->GetComponentLocation();
	const FVector EndHookSearch = Player->GetCameraComponent()->GetComponentLocation() + Player->GetCameraComponent()->GetForwardVector() * HookshotData->HookLength;

	DrawDebugLine(GetWorld(), StartHookSearch, EndHookSearch, FColor::Purple, false, 0.1f, 0.f, 3.f);

	const auto HookTrace = GetWorld()->LineTraceSingleByChannel(HookshotTarget, StartHookSearch, EndHookSearch, HookshotData->HookCollision, Parameters, FCollisionResponseParams());
	if (HookTrace)
	{
		const FVector TargetOffset = Player->GetCameraComponent()->GetForwardVector() * 50.f + FVector(0, 0, 100.f);
		TargetLocation = HookshotTarget.Location - TargetOffset;
		bIsUsingHookshot = true;
		Player->MovePlayer(TargetLocation, false, HookshotTarget.Distance/HookshotData->HookshotSpeed);
		return Eps_LeaveAiming;
	}
	
	return Player->GetCurrentState();
}

EPlayerState UMyHookshotComponent::LeaveAiming(AMyCharacter* Player)
{
	if (bIsUsingHookshot && (Player->GetActorLocation() - TargetLocation).Length() < 10.f)
		bIsUsingHookshot = false;
	
	SetAnimation(Player);

	if (!bIsUsingHookshot && Player->GetSavedState() == Eps_Sprinting)
		return Eps_Walking;
	
	if (!bIsUsingHookshot)
		return Player->GetSavedState();

	return Player->GetCurrentState();
}

void UMyHookshotComponent::SetAnimation(AMyCharacter* Player)
{
	if (bIsUsingHookshot)
		Player->SetNewAnimation(Ecmm_LeavingAim);
	else if (Player->GetIsMidAir() && !Player->GetIsExhausted())
		Player->SetNewAnimation(Ecmm_Jumping);
}

void UMyHookshotComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HookshotData)
		UE_LOG(LogTemp, Error, TEXT("MyHookshotComponent.cpp - Data Asset missing!"))
}

void UMyHookshotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}