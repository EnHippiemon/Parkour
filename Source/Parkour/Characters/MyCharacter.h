#pragma once

#include "CoreMinimal.h"
#include "MyMovementModeComponent.h"
#include "../Input/MyPlayerInput.h"
#include "Engine/DataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyCharacter.generated.h"

// SOMETHING IS WRONG WITH CLIMBING ENUM
// Shows 6 (CLIMBING) when it's supposed to be 0 (walking)
// and vice versa.

// Add climbing energy:
// Left click to push yourself and get more energy while climbing.
// The amount of energy that is restored gets smaller the longer you stay climbing.
// Add a timer, similar to the one on wall behind player check.
// The energy is drained faster the longer you stay climbing.
// Which means you need to spam energy button after a while.
// ? If the wall normal shows that you are upside down, the energy
// depletes faster? And vice versa? 

// After mentoring with Martin:
//
// 2:
// - Create an obstacle course, like a pyramid
// - Remove unnecessary text in portfolio
// - Create a self reflection / retrospect tab in portfolio instead.
// - Mention FindClimbRotation? No longer using goto but a for loop instead.
// - Where could I put the PNGs? 
// - Data classes/structs that contain the variables from h-file.
// - Maybe abstract more (create a state manager etc. All states have their
//   own scripts.
//    
// 1:
// - Situational cameras. Such as:
//   - When climbing and there is a wall behind that you could jump to,
//	   zoom out so you can see them both clearer
//	 - When running without camera input, start lerping to character's
//	   forward vector
// - Find a way to relay my project in a neat way for my portfolio
// - Somehow convey what kind of movement (what character animation)
//	 would be appropriate.
//	 - Running
//	 - Walking
//	 - Idle
//	 - Climbing
//	 - Ledge climbing
//	 - Wall jumping
//	 - Running up wall
//	 - Side jump on wall
//	 - Slide down wall 
// - Jumping on pillars
// - Remove magic numbers to a reasonable extent 
// - Cinematic camera?

// DELTA TIME?
// - Double check so that I use delta time with everything happening
//   over time, such as lerping and +=. 

// Smoother camera:
// - Make the offset at which the camera attached to the spring arm
//	 lerp towards the position that is now. Makes sure there is
//	 never a jagged camera. 

// - Make a fancy function for the floor angle & movement energy
// - ? Separate camera when sprinting for a long time, or in special areas ?
// - I have added Velocity length to sprint input. Double check so it doesn't bug.
// - Add jump while climbing without moving with WASD to jump straight out from wall normal. 
// - Make movement speed (s)lerp instead of being instant.
// - 

// Backward jump implementation:
// NEW IMPLEMENTATION:
// Linetrace to wall
// Get the wall's normal vector
// When jumping (no WASD), jump in the wall's normal vector's forward vector
// Increase the steepness allowed to backward jump
// v Use the line tracing for movement speed.
// v Check if they are shorter than a specific angle.
// v If they are, you are allowed to do a backward jump.
// ? Maybe only check the middle line trace, to make sure the character is at the right angle
// Check if input relative to GetForwardVector is negative.
// This activates a bool bCanJumpBackward
// Put said bool in the Jump function
// From the jump function, add velocity (and change direction of character)

// Movement speed and energy depletion from floor angle:
// Make the line trace length's max and min values into 0-1
// Use that to scale both speed and energy depletion seamlessly

// Walk on top of wall / jump on pillars:
// - Raycast shortly in front to ground and straight below. If not running, stop forward
//	 movement and only allow rotation. If straight below is hitting a pillar
// - Linetrace below character and forward. 
// - Several linetraces that span from below to below & slightly in front, look for
//   pillar to stand on. If pillar is found, move to that location (could in future be
//   implemented like in Assassin's Creed, where the character is jump-running or jumping to
//	 grab a ledge).
// - If a trace is found slightly in front (meaning character is on a wall, looking sideways),
//	 then allow movement in that direction. 
// - Running and raycast shortly in front can't see floor, do the linetrace below character
//	 and forward. If pillar is found, automatically perform jump. Disable character rotation?

// Known issues:
// - Currently the player automatically stops sprinting after aiming. Otherwise you don't need
//   to hold down shift to run.
// 
// - It might be difficult for the player to do wall jumping
//	 atm. Should I implement so that you just need to jump and
//   not in any direction to jump backwards?
// 
// - If jumping to side close to ground while climbing, it
//   doesn't set movement to walking
//
// - When wall jumping, continually add velocity to not lose momentum.
//   If landed or is climbing, the force stops.

enum ECurrentAnimation : int;
/* Forward Declaration */
	/* Data Assets */
	class UHookshotDataAsset;
	// class UClimbMovementDataAsset;
	class UGroundMovementDataAsset;
	class UEnergyDataAsset;
	class UJumpDataAsset;

	/* Components */
	class UMySpringArmComponent;
	class UMyCameraComponent;
	class UMyMovementModeComponent;
	class UMyClimbComponent;
	class UMyHookshotComponent;

// Needs to be UENUM if using Blueprints
UENUM(BlueprintType)
enum EPlayerState
{
	Eps_Walking,
	Eps_Sprinting,
	Eps_Idle,
	Eps_Aiming,
	Eps_LeaveAiming,
	Eps_UseHookshot,
	Eps_Climbing
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChanged, EPlayerState, NewState);

// DELETE THIS?
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCanJumpBackChanged, bool, CanJumpBack);

UCLASS()
class PARKOUR_API AMyCharacter : public AMyPlayerInput
{
	GENERATED_BODY()

public:	
	AMyCharacter();

	UPROPERTY(BlueprintAssignable)
	FOnStateChanged OnStateChanged;
	UPROPERTY(BlueprintAssignable)
	FOnCanJumpBackChanged OnCanJumpBackChanged;
	
	void SetSavedState(EPlayerState NewState) { SavedState = NewState; }
	void SetNewAnimation(ECurrentAnimation NewMode) const { MyAnimationComponent->SetCurrentAnimation(NewMode); }
	
	UFUNCTION(BlueprintCallable)
	float GetMovementEnergy() { return MovementEnergy; }
	float GetSlowMotionTimeDilation() const { return SlowMotionDilation; }
	bool GetWallIsInFront();
	// bool GetIsUsingHookshot() const { return bIsUsingHookshot; }

	FVector GetLocation() const { return GetActorLocation(); }
	UMyMovementModeComponent* GetMovementModeComponent() const { return MyAnimationComponent; }
	UMyCameraComponent* GetCameraComponent() const { return CameraComponent; }
	UMyHookshotComponent* GetHookshotComponent() const { return HookshotComponent; }
	EPlayerState GetCurrentState() const { return CurrentState; }
	EPlayerState GetSavedState() const { return SavedState; }

	void MovePlayer(const FVector& Destination, const bool EaseInOut, const float Duration);
	void SetDeceleration(const float Value) const { GetCharacterMovement()->BrakingDecelerationFlying = Value; }
	void SetMovementMode(const EMovementMode NewMode) const { GetCharacterMovement()->MovementMode = NewMode; }
	bool GetIsMidAir() const;
	bool GetIsExhausted() const { return bIsExhausted; }

	ECollisionChannel GetBlockCollision() { return BlockAllCollision; }

private:
#pragma region ---------- VARIABLES -----------
	
	/* Time dilation */
		UPROPERTY(EditDefaultsOnly, Category=TimeDilation)
		float SlowMotionDilation = 0.05f;
		
	/* Hookshot */ 
		// bool bIsUsingHookshot = false;
		// FVector TargetLocation;

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
			// ??? Impulse or velocity ??? 

			UPROPERTY(EditDefaultsOnly, Category="Climbing|Jump")
			float ClimbJumpOutImpulseUp = 55000.f;
			UPROPERTY(EditDefaultsOnly, Category="Climbing|Jump")
			float ClimbJumpOutImpulseBack = 60000.f;
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
			// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
			// UClimbMovementDataAsset* ClimbData;
			// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
			// UHookshotDataAsset* HookshotData;
	
			// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAsset, meta = (AllowPrivateAccess = "true"))
			// UHookshotDataAsset* JumpData;

#pragma endregion
	
#pragma region ---------- FUNCTIONS -----------
	/* Player states */
		void PlayerStateSwitch();
	
	/* Character movement */
			void SetMovementMode();
	
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
			void SetTimeDilation();

		/* Energy */
			void CheckExhaustion();
			void EnergyUsage();
	
		/* Jumping */
			virtual void HandleJumpInput() override;
			virtual void Landed(const FHitResult& Hit) override;
			bool GetCanJumpBackwards() const;
			void CheckIfFalling();
		
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