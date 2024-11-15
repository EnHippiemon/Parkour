#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyPlayerInput.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class PARKOUR_API AMyPlayerInput : public ACharacter
{
	GENERATED_BODY()

public:	
	AMyPlayerInput();

protected:
	/* ---------- VARIABLES ----------- */

	/* Movement */
		FVector CharacterMovement;
		FVector2D CameraMovement;
	
	/* Movement without input */
		FLatentActionInfo LatentActionInfo;


	/* ---------- FUNCTIONS ----------- */
	
	/* Character movement */
		/* Basic character movement */
			float GetMovementForward() const { return CharacterMovementForward; }
			float GetMovementSideways() const { return CharacterMovementSideways; }
	
			float GetCameraMovementX() const { return MouseMovementX; }
			float GetCameraMovementY() const { return MouseMovementY; }
	
		/* Speed changes */
			virtual void HandleSprintInput() {}
			virtual void HandleSprintStop() {}

		/* Jumping */
			virtual void HandleJumpInput() {}
			virtual void Landed(const FHitResult& Hit) override;
		
		/* Climbing */
			virtual void CancelAction() {}
	
	/* Camera movement */
		virtual void HandleSecondaryActionInput() {}
		virtual void HandleSecondaryActionStop() {}

	/* Hookshot */ 
		virtual void HandleActionInput() {}
	
	/* Inherited functions */
		virtual void BeginPlay() override;
		virtual void Tick(float DeltaTime) override;
		virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	/* ---------- Attach components ---------- */
	
		UPROPERTY(VisibleAnywhere, Category=Camera)
		USpringArmComponent* SpringArm;
		UPROPERTY(VisibleAnywhere, Category=Camera)
		UCameraComponent* FollowCamera;

private:
	/* Basic movement */
		/* Character movement */
		void HandleForwardMovementInput(const float Value);
		void HandleSidewaysMovementInput(const float Value);

		float CharacterMovementForward = 0.f;
		float CharacterMovementSideways = 0.f;

		/* Camera movement */
		void HandleCameraInputX(const float Value);
		void HandleCameraInputY(const float Value);

		float MouseMovementX = 0.f;
		float MouseMovementY = 0.f;
};