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

// Backward jump implementation:
// v Use the line tracing for movement speed.
// v Check if they are shorter than a specific angle.
// v If they are, you are allowed to do a backward jump.
// ? Maybe only check the middle line trace, to make sure the character is at the right angle
// Check if input relative to GetForwardVector is negative.
// This activates a bool bCanJumpBackward
// Put said bool in the Jump function
// From the jump function, add velocity (and change direction of character)

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
	Eps_Aiming,
	Eps_LeaveAiming,
	Eps_UseHookshot,
	Eps_Idle,
	Eps_Climbing
};

UCLASS()
class PARKOUR_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* FollowCamera;

	// Needs to be UPROPERTY if using Blueprints 
	TEnumAsByte<EPlayerState> CurrentState;

	// Save current state for later 
	TEnumAsByte<EPlayerState> SavedState;
	
public:	
	AMyCharacter();
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly)
	float StandardCameraSpeed = 5000.f;

	UPROPERTY(EditDefaultsOnly)
	float AimCameraSpeed = 50000.f;
		
	UPROPERTY(EditDefaultsOnly)
	float AimEnergyDepletionSpeed = 7.5f;
		
	UPROPERTY(EditDefaultsOnly)
	float StandardSpringArmLength = 400.f;

	UPROPERTY(EditDefaultsOnly)
	float SprintingSpringArmLength = 600.f;

	UPROPERTY(EditDefaultsOnly)
	float StopAimingSpringArmLength = 400.f;

	UPROPERTY(EditDefaultsOnly)
	float SpringArmSwitchSpeed = 0.05f;

	UPROPERTY(EditDefaultsOnly)
	float NormalCameraSwitchSpeed = 0.02f;

	UPROPERTY(EditDefaultsOnly)
	float StillFieldOfView = 60.f;

	UPROPERTY(EditDefaultsOnly)
	float WalkingFieldOfView = 70.f;

	UPROPERTY(EditDefaultsOnly)
	float SprintingFieldOfView = 125.f;

	UPROPERTY(EditDefaultsOnly)
	float SprintFOVSpeed = 0.3f;

	UPROPERTY(EditDefaultsOnly)
	float AimingFieldOfView = 70.f;

	UPROPERTY(EditDefaultsOnly)
	FVector AimingCameraOffset = FVector(0.f, 50.f, -10.f);

	UPROPERTY(EditDefaultsOnly)
	float AimingCameraMoveSpeed = 0.3f;

	// Higher is slower
	UPROPERTY(EditDefaultsOnly)
	float HookshotSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly)
	float JumpImpulseUp = 50000.f;

	UPROPERTY(EditDefaultsOnly)
	float JumpImpulseBack = 50000.f;

	UPROPERTY(BlueprintReadOnly)
	float MovementEnergy = 1.00f;

private:
	float CharacterMovementForward = 0.f;
	float CharacterMovementSideways = 0.f;
	float MovementSpeedPercent = 1.00f;

	float FloorAngle = 1.00f;
	float ExhaustionSpeed = 0.5f;
	
	float MouseMovementX = 0.f;
	float MouseMovementY = 0.f;
	
	float CurrentCameraSpeed;
	float StandardRotationRate = 500.f;
	float AimRotationRate = 30000.f;

	float HookLength = 1200.f;

	float TimeSinceMoved = 0.f;
	float CantClimbTimer = 0.f;
	
	FVector CharacterMovement;
	FVector2D CameraMovement;
	FVector TargetLocation;

	bool bIsExhausted = false;
	bool bIsUsingHookShot = false;
	bool bHasBackwardJumpAngle = false;
	bool bIsTurningBackward = false;

	void PlayerStateSwitch();
	
	// Character movement
	void HandleForwardInput(const float Value);
	void HandleSidewaysInput(const float Value);
	void MovementOutput();
	void CheckFloorAngle();
	void CheckExhaustion();
	void HandleJumpInput();
	void HandleSprintInput();
	void HandleSprintStop();
	bool BCanJumpBackwards() const;
	void CheckWallClimb();
	void StopClimbing();

	// Camera movement 
	void HandleMouseInputX(const float Value);
	void HandleMouseInputY(const float Value);
	void CameraMovementOutput();
	void CheckIdleness();
	void HandleAimInput();
	void HandleAimStop();

	// Interactions 
	void LookForHook();
	void MoveToLocation(const FLatentActionInfo&, const float) const;

	template <typename T1, typename T2>
	static void MyLerp(T1& A, T2 B, const float Alpha);
};