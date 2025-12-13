#pragma once

#include "CoreMinimal.h"
#include "DataTypes/ItemStruct.h"
#include "DataTypes/PlantInfoStruct.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtilitiesLibrary.generated.h"
struct FSlotStruct;
class UInventoryComponent;

UCLASS()
class ROOFTOPGARDEN_API UUtilitiesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static bool GetLookAtHitActor(const UObject* WorldContextObject, ECollisionChannel TraceChannel, AActor*& HitActor,
	                              bool bDrawDebug = false);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static bool GetLookAtOutHit(const UObject* WorldContextObject, ECollisionChannel TraceChannel, FHitResult& HitResult,
	                     bool bDrawDebug);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void GetPlayerLookAt(const UObject* WorldContextObject,const APlayerController* PlayerController,
		FVector& StartLocation, FVector& Direction);
	UFUNCTION(BlueprintCallable)
	static void SetTraceDistance(float NewDistance);
	UFUNCTION(BlueprintCallable)
	static void AttachComponentsSocketToSocket(USceneComponent* Target, USceneComponent* Parent, FName TargetSocketName,
	                                           FName ParentSocketName, bool bScaleToParentSocket);
	// Makes the given object look at the camera
	UFUNCTION(BlueprintCallable, Category = "Utility")
	static void LookAtCamera(USceneComponent* TargetComponent, bool bLockPitch = false, bool bLockRoll = false);

	UFUNCTION(BlueprintCallable, Category = "Utility")
	static void LookAtCameraCapped(USceneComponent* TargetComponent, bool bLockPitch = false, bool bLockRoll = false);

	// Initialize the cached camera manager
	UFUNCTION(BlueprintCallable, Category = "Utility")
	static void InitializeCameraManager(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "DataTables")
	static bool GetActivePlantInfoStruct(UInventoryComponent* InventoryComponent, FPlantInfoStruct& PlantStruct);

	UFUNCTION(BlueprintCallable, Category = "DataTables")
	static bool GetActivePlantInfoStructFromSlot(FSlotStruct SlotStruct, FPlantInfoStruct& PlantStruct);
	
	UFUNCTION(BlueprintCallable, Category = "DataTables")
	static bool GetActiveItemInfoStruct(UInventoryComponent* InventoryComponent, FItemStruct& ItemStruct);

	UFUNCTION(BlueprintCallable, Category = "DataTables")
	static bool GetActiveItemInfoStructFromSlot(FSlotStruct SlotStruct, FItemStruct& ItemStruct);

	UFUNCTION(BlueprintCallable, Category = "DataTables")
	static bool GetActiveItemInfoStructFromID(FName ItemID, FItemStruct& ItemStruct);

	UFUNCTION(BlueprintCallable)
	static FText ToSentenceCase(const FString& InputStr);
private:
	static FHitResult LastHitResult;
	static int64 LastFrame;
	static float TraceDistance;
	static TObjectPtr<APlayerCameraManager> CachedCameraManager;
};
