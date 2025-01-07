#include "MyPlayerInput.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Parkour/Characters/MyCameraComponent.h"
#include "Parkour/Characters/MySpringArmComponent.h"

AMyPlayerInput::AMyPlayerInput()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<UMySpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetCapsuleComponent());

	CameraComponent = CreateDefaultSubobject<UMyCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArm);
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AMyPlayerInput::HandleForwardMovementInput(const float Value)
{
	CharacterMovementForward = Value * GetWorld()->DeltaTimeSeconds;
}

void AMyPlayerInput::HandleSidewaysMovementInput(const float Value)
{
	CharacterMovementSideways = Value * GetWorld()->DeltaTimeSeconds;
}

void AMyPlayerInput::SetCameraInputVector()
{
	CameraMovement = FVector2D(MouseMovementX, MouseMovementY);
}

void AMyPlayerInput::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
}

void AMyPlayerInput::HandleCameraInputX(const float Value)
{
	MouseMovementX = Value * GetWorld()->DeltaTimeSeconds;
}

void AMyPlayerInput::HandleCameraInputY(const float Value)
{
	MouseMovementY = Value * GetWorld()->DeltaTimeSeconds;
}

void AMyPlayerInput::BeginPlay()
{
	Super::BeginPlay();
	
	LatentActionInfo.CallbackTarget = this;
}

void AMyPlayerInput::Tick(float const DeltaTime)
{
	Super::Tick(DeltaTime);

	SetCameraInputVector();
}

void AMyPlayerInput::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Forwards", this, &AMyPlayerInput::HandleForwardMovementInput);
	PlayerInputComponent->BindAxis("Sideways", this, &AMyPlayerInput::HandleSidewaysMovementInput);
	PlayerInputComponent->BindAxis("MouseX", this, &AMyPlayerInput::HandleCameraInputX);
	PlayerInputComponent->BindAxis("MouseY", this, &AMyPlayerInput::HandleCameraInputY);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyPlayerInput::HandleJumpInput);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyPlayerInput::HandleSprintInput);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyPlayerInput::HandleSprintStop);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMyPlayerInput::HandleSecondaryActionInput);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMyPlayerInput::HandleSecondaryActionStop);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyPlayerInput::HandleActionInput);
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AMyPlayerInput::CancelAction);
}