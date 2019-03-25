// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Public/DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();

	FindPhysicsHandleComponent();
	Input = GetOwner()->FindComponentByClass<UInputComponent>();
	SetupInputComponent();
}

void UGrabber::SetupInputComponent()
{
	if (Input) {
		Input->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		Input->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Missing InputComponent on %s"), *GetOwner()->GetName());
	}
}
void UGrabber::FindPhysicsHandleComponent()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (!PhysicsHandle) {
		UE_LOG(LogTemp, Error, TEXT("Missing PhysicsHandleComponent on %s"), *GetOwner()->GetName());
	}
}
void UGrabber::Grab() {
	FHitResult Hit = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = Hit.GetComponent();
	AActor* ActorHit = Hit.GetActor();

	if (ActorHit && PhysicsHandle) {
		PhysicsHandle->GrabComponent(
			ComponentToGrab,
			NAME_None, // no bones needed
			ComponentToGrab->GetOwner()->GetActorLocation(),
			true // allow rotation
		);
	}
}

void UGrabber::Release() {
	if(PhysicsHandle)
		PhysicsHandle->ReleaseComponent();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle)
		return;
	if (PhysicsHandle->GrabbedComponent) {
		PhysicsHandle->SetTargetLocation(GetReachLineEnd());
	}
}

const FVector UGrabber::GetReachLineStart() {
	FRotator PlayerRotation;
	FVector PlayerLocation;
	auto FirstPlayerController = GetWorld()->GetFirstPlayerController();
	if(FirstPlayerController)
		FirstPlayerController->GetPlayerViewPoint(PlayerLocation, PlayerRotation);
	return PlayerLocation;
}
const FVector UGrabber::GetReachLineEnd() {
	FRotator PlayerRotation;
	FVector PlayerLocation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(PlayerLocation, PlayerRotation);
	return PlayerLocation + PlayerRotation.Vector() * Reach;
}

const FHitResult UGrabber::GetFirstPhysicsBodyInReach() {
	/// setup query parameters
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByObjectType(
		HitResult,
		GetReachLineStart(),
		GetReachLineEnd(),
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);
	return HitResult;
}

