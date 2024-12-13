#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyPlayerInput.generated.h"

class UMySpringArmComponent;
class UMyCameraComponent;
class USceneComponent;
class UMyMovementModeComponent;

UCLASS()
class PARKOUR_API AMyPlayerInput : public ACharacter
{
	GENERATED_BODY()

public:	
	AMyPlayerInput();

	FVector GetMovementInput() const { return CharacterMovement; }
	FVector2D GetCameraInput() const { return CameraMovement; }
	UMyCameraComponent* GetCamera() const { return CameraComponent; }

	/* Basic character movement */
	float GetMovementForward() const { return CharacterMovementForward; }
	float GetMovementSideways() const { return CharacterMovementSideways; }
	
protected:
	/* ---------- VARIABLES ----------- */

	/* Movement */
		FVector CharacterMovement;
	
	/* Movement without input */
		FLatentActionInfo LatentActionInfo;


	/* ---------- FUNCTIONS ----------- */
	
	/* Character movement */
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

		// Necessary for details panel in blueprint 
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UMySpringArmComponent* SpringArm;
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UMyCameraComponent* CameraComponent;

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
		void SetCameraInputVector();

		float MouseMovementX = 0.f;
		float MouseMovementY = 0.f;
		FVector2D CameraMovement;
};