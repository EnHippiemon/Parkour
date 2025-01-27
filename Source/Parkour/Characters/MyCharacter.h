#pragma once

#include "CoreMinimal.h"
#include "MyMovementModeComponent.h"
#include "../Input/MyPlayerInput.h"
#include "Engine/DataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyCharacter.generated.h"

/* Forward Declaration */
	/* Enums */
		enum ECurrentAnimation : int;

	/* Data Assets */
		class UHookshotDataAsset;
		class UGroundMovementDataAsset;
		class UEnergyDataAsset;
		class UJumpDataAsset;

	/* Components */
		class UMySpringArmComponent;
		class UMyCameraComponent;
		class UMyMovementModeComponent;
		class UMyClimbComponent;
		class UMyHookshotComponent;

UENUM(BlueprintType)
enum EPlayerState
{
	Eps_Walking,
	Eps_Sprinting,
	Eps_Idle,
	Eps_Aiming,
	Eps_LeaveAiming,
	Eps_Climbing
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChanged, EPlayerState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopAim);

UCLASS()
class PARKOUR_API AMyCharacter : public AMyPlayerInput
{
	GENERATED_BODY()

public:	
	AMyCharacter();

	/* Delegates */
		UPROPERTY(BlueprintAssignable)
		FOnStateChanged OnStateChanged;
		UPROPERTY(BlueprintAssignable)
		FOnAim OnAim;
		UPROPERTY(BlueprintAssignable)
		FOnStopAim OnStopAim;

#pragma region ---------- PUBLIC FUNCTIONS ------------
	/* Get components */
		UFUNCTION(BlueprintCallable)
		UMyMovementModeComponent* GetMovementModeComponent() const { return MyAnimationComponent; }
		UMyCameraComponent* GetCameraComponent() const { return CameraComponent; }
		UMyHookshotComponent* GetHookshotComponent() const { return HookshotComponent; }
	
	/* Setters */
		void SetDeceleration(const float Value) const { GetCharacterMovement()->BrakingDecelerationFlying = Value; }
		void SetMovementMode(const EMovementMode NewMode) const { GetCharacterMovement()->MovementMode = NewMode; }
		void SetSavedState(EPlayerState NewState) { SavedState = NewState; }
		void SetNewAnimation(ECurrentAnimation NewMode) const { MyAnimationComponent->SetCurrentAnimation(NewMode); }

		void MovePlayer(const FVector& Destination, const bool EaseInOut, const float Duration);
		void IncreaseEnergy(const float Amount) { MovementEnergy += Amount; }
	
	/* Getters */
		bool GetIsMidAir() const;
		bool GetIsExhausted() const { return bIsExhausted; }
		bool GetWallIsInFront() const;

		FVector GetLocation() const { return GetActorLocation(); }

		UFUNCTION(BlueprintCallable)
		float GetMovementEnergy() { return MovementEnergy; }
	
		ECollisionChannel GetBlockCollision() { return BlockAllCollision; }

		EPlayerState GetCurrentState() const { return CurrentState; }
		EPlayerState GetSavedState() const { return SavedState; }
#pragma endregion 

private:
#pragma region ---------- VARIABLES -----------

	/* Character Movement */
		/* Speed */
			float TargetMovementSpeed = 600.f;
			bool bShouldStopMovementOverTime = false;

		/* Energy */
			float MovementEnergy = 1.00f;
			float MovementSpeedPercent = 1.00f;
			float FloorAngle = 1.00f;
			bool bIsExhausted = false;
			bool bCanGainEnergy = true;

		/* Jumping */
			/* Climb jump */
			UPROPERTY(EditDefaultsOnly, Category="Climbing|Jump")
			float VelocityClimbJumpOutUp = 550.f;
			UPROPERTY(EditDefaultsOnly, Category="Climbing|Jump")
			float VelocityClimbJumpOutBack = 600.f;

			/* Sliding */
				bool bIsSlidingDown = false;
	
		/* Sprinting */
			FTimerHandle TimerRunningUpWall;
			FVector RunningUpWallEndLocation;
			UPROPERTY()
			AActor* FoundWall; 
			bool bIsNearingWall = false;
			bool bHasReachedWallWhileSprinting = false;

		/* Idle */
			float TimeSinceMoved = 0.f;
	
	/* Player States */ 
		// Needs to be UPROPERTY if using Blueprints 
		TEnumAsByte<EPlayerState> CurrentState;
		// Save current state for later use 
		TEnumAsByte<EPlayerState> SavedState;
		// Previous state, to check if it has been changed
		TEnumAsByte<EPlayerState> PreviousState;

	/* Collisions */ 
		UPROPERTY(EditDefaultsOnly, Category=Energy)
		TEnumAsByte<ECollisionChannel> BlockAllCollision;
		
	/* Attachments */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
		UMyMovementModeComponent* MyAnimationComponent;
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MovementComponent, meta = (AllowPrivateAccess = "true"))
		UMyClimbComponent* ClimbComponent;
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MovementComponent, meta = (AllowPrivateAccess = "true"))
		UMyHookshotComponent* HookshotComponent;

		/* Data assets */
			UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
			UGroundMovementDataAsset* GroundMovementData;
			UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
			UEnergyDataAsset* EnergyData;

#pragma endregion
	
#pragma region ---------- PRIVATE FUNCTIONS -----------
	/* Player states */
		void PlayerStateSwitch();
	
	/* Character movement */
		/* Basic character movement */
			void MovementOutput();
			void SetPlayerVelocity(const FVector& Value) const;

		/* Speed changes */
			void CheckFloorAngle();
			virtual void HandleSprintInput() override;
			virtual void HandleSprintStop() override;
			void SetMovementSpeed(const float TargetSpeed) const;
			void StopMovementOverTime();
			void CheckShouldStopMovementOverTime();

		/* Energy */
			void CheckExhaustion();
			void EnergyUsage();
	
		/* Jumping */
			virtual void HandleJumpInput() override;
			virtual void Landed(const FHitResult& Hit) override;
			bool GetCanJumpBackwards() const;
			void CheckIfFalling() const;
		
		/* Climbing */
			virtual void CancelAction() override;

			/* Sliding */
				void DecideIfShouldSlide();

		/* Running */ 
			void RunUpToWall();
			void RunningUpWall();

	/* Camera movement */
		/* Camera changes */
			void CheckIdleness();
			virtual void HandleSecondaryActionInput() override;
			virtual void HandleSecondaryActionStop() override;

	/* Hookshot */
		virtual void HandleActionInput() override;

	/* Inherited functions */
		virtual void BeginPlay() override;
		virtual void Tick(float DeltaTime) override;
#pragma endregion
};