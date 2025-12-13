// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "SaveableComponent.h"
#include "DataTypes/SaveDataStructs/VariantDataStruct.h"
#include "InventoryComponent.generated.h"

enum class EItemType : uint8;
struct FSlotStruct;
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROOFTOPGARDEN_API UInventoryComponent : public USaveableComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, int32, IndexModified);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnInventoryChanged OnInventoryChanged;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddInventoryFailed, FText, FailText);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnAddInventoryFailed OnAddInventoryFailed;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveSlotChanged, int32, ActiveSlotIdx);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnActiveSlotChanged OnActiveSlotChanged;

	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "ItemActor"))
	bool AddToInventory(const FName& ItemID, int32 Quantity, AActor* ItemActor = nullptr);

	UFUNCTION(BlueprintCallable)
	bool AddIDToInventory(const FName& ItemID, int32 Quantity);

	UFUNCTION(BlueprintCallable)
	bool RemoveFromActiveSlot(int32 Quantity, bool bDestroyItem = true);

	UFUNCTION(BlueprintCallable)
	bool RemoveFromSlot(int32 SlotIdx, int32 Quantity, bool bDestroyItem = true);
	
	UFUNCTION(BlueprintCallable)
	void SetActiveSlot(int32 NewActiveSlot);

	UFUNCTION(BlueprintCallable)
	bool HasItemsOfType(EItemType ItemType, int32 Quantity) const;

	UFUNCTION(BlueprintCallable)
	bool RemoveItemsOfType(EItemType ItemType, int32 Quantity);
	
	UFUNCTION(BlueprintCallable)
	bool GetSlotInfo(int32 SlotIndex, FSlotStruct& OutSlotInfo) const;

	UFUNCTION(BlueprintCallable)
	void GetActiveSlotInfo(FSlotStruct& SlotStruct) const;

	UFUNCTION(BlueprintCallable)
	void GetActiveSlotActor(bool& bIsValid,AActor*& OutActor) const;

	UFUNCTION(BlueprintCallable)
	bool HasInventorySpace() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ActiveSlot = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 InventorySize = 8;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	TArray<FSlotStruct> Content;

	UPROPERTY()
	TArray<FVariantDataStruct> ContentToLoad;
	
	UPROPERTY()
	ACharacter* OwnerCharacter;

	void SpawnItemActor(FSlotStruct& ItemInfo) const;
	void SetItemInfoContent(const FSlotStruct& ItemInfo, const int32& Index, bool bAttachItem = true);
	void AttachActorToOwner(AActor* ObjectActor) const;

	virtual void LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData) override;
	virtual void SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData) override;
	virtual void RestoreRelationships_Implementation(const TMap<FGuid, AActor*>& ActorMap) override;
};
