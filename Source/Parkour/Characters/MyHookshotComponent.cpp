#include "../Characters/MyHookshotComponent.h"

#include "MyCameraComponent.h"
#include "MyCharacter.h"
#include "DataAssets/HookshotDataAsset.h"
#include "DataAssets/GroundMovementDataAsset.h"
#include "Kismet/GameplayStatics.h"

UMyHookshotComponent::UMyHookshotComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UHookshotDataAsset> Hookshot(TEXT("/Game/Player/DataAssets/HookshotDataAsset"));
	if (Hookshot.Object)
		HookshotData = Hookshot.Object;
	static ConstructorHelpers::FObjectFinder<UGroundMovementDataAsset> GroundMovement(TEXT("/Game/Player/DataAssets/GroundMovementDataAsset"));
	if (GroundMovement.Object)
		GroundMovementData = GroundMovement.Object;
}

EPlayerState UMyHookshotComponent::UseHookshot(AMyCharacter* ThisPlayer)
{
	if (ThisPlayer->GetCurrentState() != Eps_Aiming)
		return ThisPlayer->GetCurrentState();

	FHitResult HookshotTarget;
	FCollisionQueryParams Parameters;
	Parameters.AddIgnoredActor(ThisPlayer);

	const FVector StartHookSearch = ThisPlayer->GetCameraComponent()->GetComponentLocation();
	const FVector EndHookSearch = ThisPlayer->GetCameraComponent()->GetComponentLocation() + ThisPlayer->GetCameraComponent()->GetForwardVector() * HookshotData->HookLength;

	DrawDebugLine(GetWorld(), StartHookSearch, EndHookSearch, FColor::Purple, false, 0.1f, 0.f, 3.f);

	const auto HookTrace = GetWorld()->LineTraceSingleByChannel(HookshotTarget, StartHookSearch, EndHookSearch, HookshotData->HookCollision, Parameters, FCollisionResponseParams());
	if (HookTrace)
	{
		const FVector TargetOffset = ThisPlayer->GetCameraComponent()->GetForwardVector() * 50.f + FVector(0, 0, 100.f);
		TargetLocation = HookshotTarget.Location - TargetOffset;
		bIsUsingHookshot = true;
		ThisPlayer->MovePlayer(TargetLocation, false, HookshotTarget.Distance/HookshotData->HookshotSpeed);
		return Eps_LeaveAiming;
	}
	
	return ThisPlayer->GetCurrentState();
}

EPlayerState UMyHookshotComponent::LeaveAiming(const AMyCharacter* ThisPlayer)
{
	if (Player->GetCurrentState() != Eps_LeaveAiming)
		return Player->GetCurrentState();
	
	constexpr int ReachedOffset = 15;
	if (bIsUsingHookshot && (ThisPlayer->GetActorLocation() - TargetLocation).Length() < ReachedOffset)
		bIsUsingHookshot = false;
	
	SetAnimation(ThisPlayer);

	if (!bIsUsingHookshot && ThisPlayer->GetSavedState() == Eps_Sprinting)
		return Eps_Walking;
	
	if (!bIsUsingHookshot)
		return ThisPlayer->GetSavedState();

	return ThisPlayer->GetCurrentState();
}

void UMyHookshotComponent::SetAnimation(const AMyCharacter* ThisPlayer) const
{
	if (bIsUsingHookshot)
		ThisPlayer->SetNewAnimation(Ecmm_LeavingAim);
	else if (ThisPlayer->GetIsMidAir() && !ThisPlayer->GetIsExhausted())
		ThisPlayer->SetNewAnimation(Ecmm_Jumping);
}

void UMyHookshotComponent::StateSwitch(const EPlayerState NewState)
{
	if (NewState == Eps_Aiming)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SlowMotionDilation);
		Player->GetCharacterMovement()->bOrientRotationToMovement = false;
		Player->GetCharacterMovement()->bUseControllerDesiredRotation = true;
		Player->GetCharacterMovement()->RotationRate = FRotator(0.f, GroundMovementData->AimRotationRate, 0.f);
		Player->SetNewAnimation(Ecmm_Aiming);
	}
	else if (NewState == Eps_LeaveAiming)
	{
		Player->GetCharacterMovement()->RotationRate = FRotator(0.f, GroundMovementData->StandardRotationRate, 0.f);
		Player->GetCharacterMovement()->bOrientRotationToMovement = true;
		Player->GetCharacterMovement()->bUseControllerDesiredRotation = false;
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
	}
}

void UMyHookshotComponent::SetTimeDilation()
{
	if (!Player->GetIsMidAir() && UGameplayStatics::GetGlobalTimeDilation(GetWorld()) < 1)
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1);
}

void UMyHookshotComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HookshotData)
		UE_LOG(LogTemp, Error, TEXT("MyHookshotComponent.cpp - Data Asset missing!"))

	Player = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (IsValid(Player))
		Player->OnStateChanged.AddUniqueDynamic(this, &UMyHookshotComponent::StateSwitch);
}

void UMyHookshotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}