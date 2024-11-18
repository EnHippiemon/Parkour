#pragma once

#include "CoreMinimal.h"
#include "../Input/MyPlayerInput.h"
#include "MyCharacter.generated.h"

// class USceneComponent;
// class USpringArmComponent;
// class UCameraComponent;
// class AMyPlayerInput;

// To do:
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


// Needs to be UENUM if using Blueprints
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

UCLASS()
class PARKOUR_API AMyCharacter : public AMyPlayerInput
{
	GENERATED_BODY()

public:	
	AMyCharacter();

	UFUNCTION(BlueprintCallable)
	float GetMovementEnergy() { return MovementEnergy; }
	
private:
#pragma region ---------- VARIABLES -----------
	/* Camera */
		/* Camera speed */
			UPROPERTY(EditDefaultsOnly, Category=CameraSpeed)
			float StandardCameraSpeed = 5000.f;
			UPROPERTY(EditDefaultsOnly, Category=CameraSpeed)
			float AimCameraSpeed = 50000.f;

		/* Camera position */
			UPROPERTY(EditDefaultsOnly, Category=CameraPosition)
			FVector AimingCameraOffset = FVector(0.f, 50.f, -10.f);
			
			// Decides how far to the sides the camera can move.
			UPROPERTY(EditDefaultsOnly, Category=CameraPosition)
			FVector CameraClamp = FVector(0.f, 200.f, 100.f);

			float CurrentCameraOffsetY = 150.f;
			float CurrentCameraOffsetZ = 0.f;
		
		/* Camera positioning speed */ 
			UPROPERTY(EditDefaultsOnly, Category=CameraPositionSpeed)
			float AimingCameraTransitionAlpha = 0.3f;
			UPROPERTY(EditDefaultsOnly, Category=CameraPositionSpeed)
			float StandardRotationRate = 500.f;
			UPROPERTY(EditDefaultsOnly, Category=CameraPositionSpeed)
			float AimRotationRate = 30000.f;
		
			// Decides how quickly the camera moves from side to side. 
			UPROPERTY(EditDefaultsOnly, Category=CameraPositionSpeed)
			float CameraYDirectionSpeed = 1000.f;
		
			float CurrentCameraSpeed;

		/* Camera field of view (FOV) */ 
			UPROPERTY(EditDefaultsOnly, Category=CameraFieldOfView)
			float StillFOV = 60.f;
			UPROPERTY(EditDefaultsOnly, Category=CameraFieldOfView)
			float WalkingFOV = 70.f;
			UPROPERTY(EditDefaultsOnly, Category=CameraFieldOfView)
			float SprintingFOV = 125.f;
			UPROPERTY(EditDefaultsOnly, Category=CameraFieldOfView)
			float SprintFOVSpeed = 0.3f;
			UPROPERTY(EditDefaultsOnly, Category=CameraFieldOfView)
			float AimingFOV = 70.f;

	/* Spring Arm */
		/* Spring arm length */
			UPROPERTY(EditDefaultsOnly, Category=SpringArmLength)
			float StandardSpringArmLength = 400.f;
			UPROPERTY(EditDefaultsOnly, Category=SpringArmLength)
			float SprintingSpringArmLength = 600.f;
			UPROPERTY(EditDefaultsOnly, Category=SpringArmLength)
			float StopAimingSpringArmLength = 400.f;

		/* Spring arm extension speed */ 
			UPROPERTY(EditDefaultsOnly, Category=SpringArmExtensionSpeed)
			float SpringArmSwitchSpeed = 0.05f;
			UPROPERTY(EditDefaultsOnly, Category=SpringArmExtensionSpeed)
			float NormalCameraSwitchSpeed = 0.02f;
	
	/* Hookshot */ 
		// Higher is slower
		UPROPERTY(EditDefaultsOnly, Category=Hookshot)
		float HookshotSpeed = 1000.f;
		UPROPERTY(EditDefaultsOnly, Category=Hookshot)
		float HookLength = 1200.f;
		UPROPERTY(EditDefaultsOnly, Category=Hookshot)
		TEnumAsByte<ECollisionChannel> HookCollision;

		bool bIsUsingHookshot = false;
		FVector TargetLocation;

	/* Character Movement */
		/* Jumping */ 
			UPROPERTY(EditDefaultsOnly, Category=Jump)
			float JumpImpulseUp = 50000.f;
			UPROPERTY(EditDefaultsOnly, Category=Jump)
			float JumpImpulseBack = 50000.f;

		/* Energy */
			UPROPERTY(EditDefaultsOnly, Category=Energy)
			float AimEnergyDepletionSpeed = 7.5f;
			UPROPERTY(EditDefaultsOnly, Category=Energy)
			TEnumAsByte<ECollisionChannel> BlockAllCollision;
			UPROPERTY(EditDefaultsOnly, Category=Energy)
			float ExhaustionSpeed = 0.5f;
		
			float MovementEnergy = 1.00f;
			float MovementSpeedPercent = 1.00f;
			float FloorAngle = 1.00f;
			bool bIsExhausted = false;

		/* Climbing */
			UPROPERTY(EditDefaultsOnly, Category=Climbing)
			TEnumAsByte<ECollisionChannel> ClimbingCollision;
			UPROPERTY(EditDefaultsOnly, Category=Climbing)
			float AdjustPlayerRotationDistance = 15.f;
	
			float CantClimbTimer = 0.f;

			/* Ledge climbing */
				UPROPERTY (EditDefaultsOnly, Category=Climbing)
				FVector LedgeClimbDetectionOffset = FVector(50.f, 0.f, 100.f);
				UPROPERTY(EditDefaultsOnly, Category=Climbing)
				float LedgeClimbDuration = 1.f;
				UPROPERTY(EditDefaultsOnly, Category=Climbing)
				TEnumAsByte<ECollisionChannel> LedgeChannel;

				FVector LedgeClimbDestination; 
				bool bIsClimbingLedge = false;
	
		/* Sprinting */
			UPROPERTY(EditDefaultsOnly, Category=Sprinting)
			TEnumAsByte<ETraceTypeQuery> ObstacleTraceType;
			UPROPERTY(EditDefaultsOnly, Category=Sprinting)
			float DistanceBeforeAbleToRunUpWall = 300.f;
			UPROPERTY(EditDefaultsOnly, Category=Sprinting)
			float RunningUpWallSpeed = 20.f;
			UPROPERTY(EditDefaultsOnly, Category=Sprinting)
			float DistanceToRunUpWall = 200.f;
		
			FTimerHandle TimerRunningUpWall;
			FVector RunningUpWallEndLocation; 
			bool bIsNearingWall = false;
			bool bHasReachedWallWhileSprinting = false;

		/* Idle */
			UPROPERTY(EditDefaultsOnly, Category=Idle)
			float TimeBeforeIdle = 15.f;

			float TimeSinceMoved = 0.f;
	
	/* Player States */ 
		// Needs to be UPROPERTY if using Blueprints 
		TEnumAsByte<EPlayerState> CurrentState;
		// Save current state for later use 
		TEnumAsByte<EPlayerState> SavedState;
		// Previous state, to check if it has been changed
		TEnumAsByte<EPlayerState> PreviousState;
#pragma endregion
	
#pragma region ---------- FUNCTIONS -----------
	/* Player states */
		void PlayerStateSwitch();
	
	/* Character movement */
		/* Basic character movement */
			void MovementOutput();
			void SetPlayerVelocity(const FVector& Value) const;

		/* Speed changes */
			void CheckFloorAngle();
			void CheckExhaustion();
			virtual void HandleSprintInput() override;
			virtual void HandleSprintStop() override;

		/* Jumping */
			virtual void HandleJumpInput() override;
			virtual void Landed(const FHitResult& Hit) override;
			bool BCanJumpBackwards() const;
			bool GetIsMidAir() const;
		
		/* Climbing */
			void CheckWallClimb();
			void SetClimbRotation();
			virtual void CancelAction() override;

			/* Ledge climbing */
				void LookForLedge();

		/* Running */ 
			void RunUpToWall();
			void RunningUpWall();

	/* Camera movement */
		/* Basic camera movement */
			void CameraMovementOutput();

		/* Camera changes */
			void CheckIdleness();
			virtual void HandleSecondaryActionInput() override;
			virtual void HandleSecondaryActionStop() override;
			void SetCurrentOffset(float& Value, const float Speed, const float Clamp) const;

	/* Hookshot */
		virtual void HandleActionInput() override;

	/* Inherited functions */
		virtual void BeginPlay() override;
		virtual void Tick(float DeltaTime) override;
#pragma endregion 
};