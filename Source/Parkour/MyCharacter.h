#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

// To do:
// - Make a fancy function for the floor angle & movement energy
// - ? Separate camera when sprinting for a long time, or in special areas ?
// - I have added Velocity length to sprint input. Double check so it doesn't bug. 

// Camera Offset:
// - If the character moves towards camera's right vector
// - Followcamera offsetY -= walk direction related to camera clamped to max value. 
// - When moving to right of screen, change camera offset to left.
// - When standing to the left and moving camera to left, change camera offset to right.

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

// Climbing rotation: 
// NEW IMPLEMENTATION:
// While climbing, create a linetrace pointing forward, with an offset based on moving direction
// Get the normal off the wall using the linetrace
// Rotate player towards wall normal.

// Ledge climbing:
// Linetrace above player
// If no collision while climbing, movetotarget

// Movement speed and energy depletion from floor angle:
// Make the line trace length's max and min values into 0-1
// Use that to scale both speed and energy depletion seamlessly

// Known issues:
// - Currently the player automatically stops sprinting after aiming. Otherwise you don't need
// to hold down shift to run.
// - It might be difficult for the player to do wall jumping
// atm. Should I implement so that you just need to jump and
// not in any direction to jump backwards? 


// Needs to be UENUM if using Blueprints
enum EPlayerState
{
	Eps_Walking,
	Eps_Sprinting,
	Eps_Idle,
	Eps_Aiming,
	Eps_LeaveAiming,
	Eps_UseHookshot,
	Eps_Climbing,
	Eps_NoInput
};

UCLASS()
class PARKOUR_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:	
	AMyCharacter();

	UFUNCTION(BlueprintCallable)
	float GetMovementEnergy() { return MovementEnergy; }
	
private:
	/* ---------- VARIABLES ----------- */

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
		float CantClimbTimer = 0.f;

	/* Sprinting */
		UPROPERTY(EditDefaultsOnly, Category=Sprinting)
		TEnumAsByte<ETraceTypeQuery> ObstacleTraceType;
		UPROPERTY(EditDefaultsOnly, Category=Sprinting)
		float RunningUpWallSpeed = 20.f;
	
		FTimerHandle TimerRunningUpWall;
		bool bHasReachedWallWhileSprinting = false; 

	/* Basic movement */
		/* Idle */
			UPROPERTY(EditDefaultsOnly, Category=Idle)
			float TimeBeforeIdle = 15.f;

			float TimeSinceMoved = 0.f;

		/* Basic character movement */ 
			FVector CharacterMovement;
			float CharacterMovementForward = 0.f;
			float CharacterMovementSideways = 0.f;
		
		/* Basic camera movement */
			FVector2D CameraMovement;
			float MouseMovementX = 0.f;
			float MouseMovementY = 0.f;
	
	/* Player states */ 
		// Needs to be UPROPERTY if using Blueprints 
		TEnumAsByte<EPlayerState> CurrentState;
		// Save current state for later use 
		TEnumAsByte<EPlayerState> SavedState;


	/* ---------- FUNCTIONS ----------- */
	
	/* Player states */
		void PlayerStateSwitch();
	
	/* Character movement */
		/* Basic character movement */
			void HandleForwardInput(const float Value);
			void HandleSidewaysInput(const float Value);
			void MovementOutput();

		/* Speed changes */
			void CheckFloorAngle();
			void CheckExhaustion();
			void HandleSprintInput();
			void HandleSprintStop();

		/* Jumping */
			void HandleJumpInput();
			virtual void Landed(const FHitResult& Hit) override;
			bool BCanJumpBackwards() const;
			bool GetIsMidAir() const;
		
		/* Climbing */
			void CheckWallClimb();
			void StopClimbing();

		/* Running */ 
			void RunUpToWall();

		/* Movement without input */
			void MoveToLocation(const FLatentActionInfo&, const float) const;

	/* Camera movement */
		/* Basic camera movement */
			void HandleMouseInputX(const float Value);
			void HandleMouseInputY(const float Value);
			void CameraMovementOutput();

		/* Camera changes */
			void CheckIdleness();
			void HandleAimInput();
			void HandleAimStop();
			void SetCurrentOffset(float& Value, const float Speed, const float Clamp) const;

	/* Hookshot */ 
		void LookForHook();

	/* Static functions */ 
		template <typename T1, typename T2>
		static void MyLerp(T1& A, T2 B, const float Alpha);

	/* Inherited functions */
		virtual void BeginPlay() override;
		virtual void Tick(float DeltaTime) override;
		virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* Attach components */
		UPROPERTY(VisibleAnywhere, Category=Camera)
		USpringArmComponent* SpringArm;
		UPROPERTY(VisibleAnywhere, Category=Camera)
		UCameraComponent* FollowCamera;
};