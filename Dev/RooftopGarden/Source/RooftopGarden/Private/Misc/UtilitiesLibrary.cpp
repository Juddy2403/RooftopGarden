// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/UtilitiesLibrary.h"
#include "Components/InventoryComponent.h"
#include "DataTypes/DataTablePresets.h"
#include "DataTypes/SlotStruct.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include <regex>       
#include <string>

FHitResult UUtilitiesLibrary::LastHitResult = FHitResult();
int64 UUtilitiesLibrary::LastFrame = 0;
float UUtilitiesLibrary::TraceDistance = 200.0f;
TObjectPtr<APlayerCameraManager> UUtilitiesLibrary::CachedCameraManager = nullptr;

bool UUtilitiesLibrary::GetLookAtHitActor(const UObject* WorldContextObject, ECollisionChannel TraceChannel,
                                          AActor*& HitActor, bool bDrawDebug)
{
	FHitResult HitResult;
	const bool bHit = GetLookAtOutHit(WorldContextObject, TraceChannel, HitResult, bDrawDebug);
	if (bHit)
	{
		HitActor = HitResult.GetActor();
		return true;
	}
	HitActor = nullptr;
	return false;
}

bool UUtilitiesLibrary::GetLookAtOutHit(const UObject* WorldContextObject, ECollisionChannel TraceChannel,
										  FHitResult& HitResult, bool bDrawDebug)
{
	if (LastFrame == GFrameCounter && LastHitResult.MyItem != INDEX_NONE)
	{
		HitResult = LastHitResult;
		return true;
	}
	LastFrame = GFrameCounter;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return false;

	// getting the screen center position
	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (!PlayerController) return false;

	FVector WorldLocation, WorldDirection;
	GetPlayerLookAt(WorldContextObject, PlayerController, WorldLocation, WorldDirection);

	FHitResult OutHit;
	const FVector Start = WorldLocation;
	const FVector End = Start + WorldDirection * TraceDistance;

	const bool bHit = UKismetSystemLibrary::LineTraceSingle(
		World,
		Start,
		End,
		UEngineTypes::ConvertToTraceType(TraceChannel),
		false, // Trace complex
		TArray<AActor*>(), // Actors to ignore
		bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, // <--- THIS ENABLES DEBUG DRAW
		OutHit,
		true // Ignore self
	);
	HitResult = OutHit;
	if (bHit) LastHitResult = HitResult;
	else LastHitResult.MyItem = INDEX_NONE;
	return bHit;
}

void UUtilitiesLibrary::GetPlayerLookAt(const UObject* WorldContextObject,const APlayerController* PlayerController, 
	FVector& StartLocation, FVector& Direction)
{
	int32 ViewportX, ViewportY;
	PlayerController->GetViewportSize(ViewportX, ViewportY);
	const FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);

	// getting the world location for the screen center
	PlayerController->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, StartLocation, Direction);
}

void UUtilitiesLibrary::SetTraceDistance(float NewDistance)
{
	TraceDistance = NewDistance;
}

void UUtilitiesLibrary::AttachComponentsSocketToSocket(USceneComponent* Target, USceneComponent* Parent,
                                                       FName TargetSocketName, FName ParentSocketName,
                                                       bool bScaleToParentSocket)
{
	// Rotate in such a way that TargetSocket rotation becomes the same with ParentSocket rotation
	FVector OriginalTargetSocketLocation = Target->GetSocketLocation(TargetSocketName);
	FQuat DeltaRootQuat = Parent->GetSocketQuaternion(ParentSocketName) * Target->GetSocketQuaternion(TargetSocketName).
		Inverse();
	Target->SetWorldRotation(DeltaRootQuat * Target->GetComponentQuat(), false, nullptr,
	                         ETeleportType::TeleportPhysics);
	FVector RotationLocationOffset = OriginalTargetSocketLocation - Target->GetSocketLocation(TargetSocketName);
	Target->AddWorldOffset(RotationLocationOffset, false, nullptr, ETeleportType::TeleportPhysics);

	// Set location in such a way that TargetSocket location becomes the same with ParentSocket location
	Target->AddWorldOffset(Parent->GetSocketLocation(ParentSocketName) - Target->GetSocketLocation(TargetSocketName));

	// If scaling is needed, scale Target as specified by ParentSocket
	if (bScaleToParentSocket)
	{
		Target->SetWorldScale3D(Parent->GetSocketTransform(ParentSocketName, RTS_World).GetScale3D());
	}

	// Attach target to parent
	Target->AttachToComponent(Parent, FAttachmentTransformRules::KeepWorldTransform, ParentSocketName);
}

void UUtilitiesLibrary::LookAtCamera(USceneComponent* TargetComponent, bool bLockPitch, bool bLockRoll)
{
	if (!IsValid(TargetComponent) || !IsValid(CachedCameraManager)) return;

	const FVector CameraLocation = CachedCameraManager->GetCameraLocation();
	const FVector TargetLocation = TargetComponent->GetComponentLocation();
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, CameraLocation);

	// Optionally lock certain axes
	if (bLockPitch)
	{
		LookAtRotation.Pitch = TargetComponent->GetComponentRotation().Pitch;
	}
	if (bLockRoll)
	{
		LookAtRotation.Roll = TargetComponent->GetComponentRotation().Roll;
	}
	TargetComponent->SetWorldRotation(LookAtRotation);
}

void UUtilitiesLibrary::LookAtCameraCapped(USceneComponent* TargetComponent, bool bLockPitch, bool bLockRoll)
{
	LookAtCamera(TargetComponent, bLockPitch, bLockRoll);
	FRotator LookAtRotation = TargetComponent->GetComponentRotation();
	LookAtRotation.Yaw = FMath::RoundToFloat(LookAtRotation.Yaw / 90.f) * 90.f;
	TargetComponent->SetWorldRotation(LookAtRotation);

}

void UUtilitiesLibrary::InitializeCameraManager(APlayerController* PlayerController)
{
	if (!PlayerController) return;
	CachedCameraManager = PlayerController->PlayerCameraManager;
}

bool UUtilitiesLibrary::GetActivePlantInfoStruct(UInventoryComponent* InventoryComponent, FPlantInfoStruct& PlantStruct) 
{
	FSlotStruct SlotStruct;
	InventoryComponent->GetActiveSlotInfo(SlotStruct);
	bool Success = GetActivePlantInfoStructFromSlot(SlotStruct, PlantStruct);
	return Success;
}

bool UUtilitiesLibrary::GetActivePlantInfoStructFromSlot(FSlotStruct SlotStruct, FPlantInfoStruct& PlantStruct)
{
	if (SlotStruct.ItemID.IsNone()) return false;
	const UDataTablePresets* DataTablePresets = GetDefault<UDataTablePresets>();
	UDataTable* PlantDataTable = DataTablePresets->GetPlantDataTable();
	const auto PlantInfoStruct = PlantDataTable->FindRow<FPlantInfoStruct>(
		SlotStruct.ItemID, TEXT("Plant info struct"), true);
	if (!PlantInfoStruct) return false;
	PlantStruct = *PlantInfoStruct;
	return true;
}

bool UUtilitiesLibrary::GetActiveItemInfoStruct(UInventoryComponent* InventoryComponent, FItemStruct& ItemStruct) 
{
	FSlotStruct SlotStruct;
	InventoryComponent->GetActiveSlotInfo(SlotStruct);
	bool Success = GetActiveItemInfoStructFromSlot(SlotStruct, ItemStruct);
	return Success;
}

bool UUtilitiesLibrary::GetActiveItemInfoStructFromSlot(FSlotStruct SlotStruct, FItemStruct& ItemStruct)
{
	return GetActiveItemInfoStructFromID(SlotStruct.ItemID, ItemStruct);
}

bool UUtilitiesLibrary::GetActiveItemInfoStructFromID(FName ItemID, FItemStruct& ItemStruct)
{
	if (ItemID.IsNone()) return false;
	const UDataTablePresets* DataTablePresets = GetDefault<UDataTablePresets>();
	UDataTable* ItemDataTable = DataTablePresets->GetItemDataTable();
	const auto ItemDataStruct = ItemDataTable->FindRow<FItemStruct>(
		ItemID, TEXT("Plant info struct"), true); 
	if (!ItemDataStruct) return false;
	ItemStruct = *ItemDataStruct;
	return true;
}

FText UUtilitiesLibrary::ToSentenceCase(const FString& InputStr)
{
	std::string InputStd = TCHAR_TO_UTF8(*InputStr);

	// Regex: insert space before ANY uppercase letter that follows a lowercase letter
	std::regex Pattern("([a-z])([A-Z])");
	std::string ResultStd = std::regex_replace(InputStd, Pattern, "$1 $2");

	// Convert std::string back to FText
	FString OutputFString = UTF8_TO_TCHAR(ResultStd.c_str());
	return FText::FromString(OutputFString);
}
