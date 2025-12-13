#include "Components/InventoryComponent.h"

#include "InstancedStruct.h"
#include "DataTypes/ItemStruct.h"
#include "DataTypes/SlotStruct.h"
#include "GameFramework/Character.h"
#include "Interfaces/SaveableInterface.h"
#include "Misc/UtilitiesLibrary.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	// add inventory size amount of empty slots
	Content.SetNum(InventorySize, true);
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryComponent must be attached to an Actor of type Character"));
	}
}

bool UInventoryComponent::AddToInventory(const FName& ItemID, int32 Quantity, AActor* ItemActor)
{
	FItemStruct ItemStruct;
	const bool Success = UUtilitiesLibrary::GetActiveItemInfoStructFromID(ItemID, ItemStruct);
	if (!Success) return false;
	int FirstEmptySlotIndex = -1;
	// Tools don't stack, so if active is empty that's where it should be placed
	if (ItemStruct.ItemType == EItemType::Tool && Content[ActiveSlot].ItemID.IsNone()) FirstEmptySlotIndex = ActiveSlot;
	else
		for (int i{Content.Num() - 1}; i >= 0; --i)
		{
			// Record the first empty slot we find
			if (Content[i].ItemID.IsNone()) FirstEmptySlotIndex = i;
			// Tools don't stack, we just need the first empty slot idx
			if (ItemStruct.ItemType == EItemType::Tool) continue;
			// Check if there is one of these items in the inventory already, if so add to that stack
			if (Content[i].ItemID == ItemID)
			{
				// Do not attach an extra item since you already have it
				SetItemInfoContent({ItemID, Quantity, ItemActor, ItemStruct.ItemType}, i, false);
				OnInventoryChanged.Broadcast(i);
				return true;
			}
		}
	if (FirstEmptySlotIndex == -1)
	{
		OnAddInventoryFailed.Broadcast(FText::FromString("No inventory space"));
		return false; // inventory full
	}

	// If the active slot is empty add item there, otherwise use the first empty slot found
	int SlotToAdd = FirstEmptySlotIndex;
	if (Content[ActiveSlot].ItemID.IsNone()) SlotToAdd = ActiveSlot;
	SetItemInfoContent({ItemID, Quantity, ItemActor, ItemStruct.ItemType}, SlotToAdd);
	OnInventoryChanged.Broadcast(SlotToAdd);
	return true;
}

bool UInventoryComponent::AddIDToInventory(const FName& ItemID, int32 Quantity)
{
	return AddToInventory(ItemID, Quantity, nullptr);
}

bool UInventoryComponent::RemoveFromActiveSlot(int32 Quantity, bool bDestroyItem)
{
	return RemoveFromSlot(ActiveSlot, Quantity, bDestroyItem);
}

bool UInventoryComponent::RemoveFromSlot(int32 SlotIdx, int32 Quantity, bool bDestroyItem)
{
	// Cannot remove more than we have
	if (Content[SlotIdx].Quantity < Quantity) return false;

	Content[SlotIdx].Quantity -= Quantity;
	// If all items from stack have been removed, empty the slot
	if (Content[SlotIdx].Quantity == 0)
	{
		if (IsValid(Content[SlotIdx].Item) && bDestroyItem) Content[SlotIdx].Item->Destroy();
		if (!bDestroyItem)
		{
			Content[SlotIdx].Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			Content[SlotIdx].Item->SetActorEnableCollision(true);
			UStaticMeshComponent* Mesh = Content[SlotIdx].Item->FindComponentByClass<UStaticMeshComponent>();
			if (Mesh) Mesh->SetCastShadow(true);
		}
		Content[SlotIdx].ItemID = NAME_None;
		Content[SlotIdx].Item = nullptr;
		Content[SlotIdx].ItemType = EItemType::Default;
	}

	OnInventoryChanged.Broadcast(SlotIdx);
	return true;
}

void UInventoryComponent::SetActiveSlot(int32 NewActiveSlot)
{
	// Hide the past active object (if we have it)
	auto ActiveItem = Content[ActiveSlot].Item;
	if (IsValid(ActiveItem)) ActiveItem->SetActorHiddenInGame(true);
	ActiveSlot = FMath::Clamp(NewActiveSlot, 0, InventorySize - 1);
	// Show the new active object (if we have it)
	ActiveItem = Content[ActiveSlot].Item;
	if (IsValid(ActiveItem)) ActiveItem->SetActorHiddenInGame(false);
	OnActiveSlotChanged.Broadcast(ActiveSlot);
}

bool UInventoryComponent::HasItemsOfType(EItemType ItemType, int32 Quantity) const
{
	int32 TotalQuantity = 0;
	for (const auto& Slot : Content)
	{
		if (Slot.ItemType != ItemType) continue;
		TotalQuantity += Slot.Quantity;
		if (TotalQuantity >= Quantity) return true;
	}
	return false;
}

bool UInventoryComponent::RemoveItemsOfType(EItemType ItemType, int32 Quantity)
{
	if (!HasItemsOfType(ItemType, Quantity)) return false;
	int32 QuantityToRemove = Quantity;
	if (Content[ActiveSlot].ItemType == ItemType)
	{
		if (Content[ActiveSlot].Quantity >= QuantityToRemove)
		{
			RemoveFromActiveSlot(QuantityToRemove);
			return true;
		}
		QuantityToRemove -= Content[ActiveSlot].Quantity;
		RemoveFromActiveSlot(Content[ActiveSlot].Quantity);
		if (QuantityToRemove == 0) return true;
	}
	for (int i{}; i<= InventorySize -1; ++i)
	{
		if (i == ActiveSlot) continue;
		auto& Slot = Content[i];
		if (Slot.ItemType != ItemType) continue;
		if (Slot.Quantity >= QuantityToRemove)
		{
			RemoveFromSlot(i, QuantityToRemove, false);
			return true;
		}
		QuantityToRemove -= Slot.Quantity;
		RemoveFromSlot(ActiveSlot, Slot.Quantity, false);
		if (QuantityToRemove == 0) return true;
	}
	return false;
}


bool UInventoryComponent::GetSlotInfo(int32 SlotIndex, FSlotStruct& OutSlotInfo) const
{
	if (!Content.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Active slot index %d is out of bounds for inventory size %d"), ActiveSlot,
		       InventorySize);
		return false;
	}
	OutSlotInfo = Content[SlotIndex];
	return true;
}

void UInventoryComponent::GetActiveSlotInfo(FSlotStruct& SlotStruct) const
{
	SlotStruct = Content[ActiveSlot];
}

void UInventoryComponent::GetActiveSlotActor(bool& bIsValid, AActor*& Actor) const
{
	if (IsValid(Content[ActiveSlot].Item)) bIsValid = true;
	else bIsValid = false;
	Actor = Content[ActiveSlot].Item;
}

bool UInventoryComponent::HasInventorySpace() const
{
	for (const auto& Slot : Content)
	{
		if (Slot.Quantity == 0) return true;
	}
	return false;
}

void UInventoryComponent::SpawnItemActor(FSlotStruct& ItemInfo) const
{
	FItemStruct ItemStruct;
	bool Success = UUtilitiesLibrary::GetActiveItemInfoStructFromSlot(ItemInfo, ItemStruct);
	if (!Success) return;
	ItemInfo.Item = GetWorld()->SpawnActor(ItemStruct.ItemClass);
}

void UInventoryComponent::SetItemInfoContent(const FSlotStruct& ItemInfo, const int32& Index, bool bAttachItem)
{
	Content[Index].ItemID = ItemInfo.ItemID;
	Content[Index].Quantity += ItemInfo.Quantity;
	Content[Index].ItemType = ItemInfo.ItemType;

	// If the item actor exists, add it to the slot info and attach it
	if (IsValid(ItemInfo.Item))
	{
		// If the item actor should not be attached and it is valid, destroy it and exit
		if (!bAttachItem)
		{
			ItemInfo.Item->Destroy();
			return;
		}
		Content[Index].Item = ItemInfo.Item;
	}
	// If the item actor does not exist and it needs to be attached, spawn it
	else if (bAttachItem) SpawnItemActor(Content[Index]);
	AttachActorToOwner(Content[Index].Item);
	// Hide the item actor when its not the current active item
	if (Index != ActiveSlot) Content[Index].Item->SetActorHiddenInGame(true);
}

void UInventoryComponent::AttachActorToOwner(AActor* ObjectActor) const
{
	if (!IsValid(ObjectActor)) return;

	UUtilitiesLibrary::AttachComponentsSocketToSocket(ObjectActor->GetRootComponent(), OwnerCharacter->GetMesh(),
	                                                  FName("Handle"), FName("Grip"), false);
	ObjectActor->SetOwner(GetOwner());
	ObjectActor->SetActorEnableCollision(false);
	UStaticMeshComponent* Mesh = ObjectActor->FindComponentByClass<UStaticMeshComponent>();
	if (Mesh) Mesh->SetCastShadow(false);
}

void UInventoryComponent::LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData)
{
	for (const auto& Slot : SaveData)
	{
		FVariantDataStruct DataStruct = Slot.Get<FVariantDataStruct>();
		ContentToLoad.Add(DataStruct);
		//AddIDToInventory(SlotStruct.ItemID, SlotStruct.Quantity);
	}
}

void UInventoryComponent::SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData)
{
	OutSaveData.Reserve(Content.Num());
	for (const auto& Slot : Content)
	{
		if (Slot.ItemID == NAME_None) continue;
		FInstancedStruct SlotStruct;
		SlotStruct.InitializeAs<FVariantDataStruct>();
		FVariantDataStruct* DataStruct = SlotStruct.GetMutablePtr<FVariantDataStruct>();
		DataStruct->NameValue = Slot.ItemID;
		DataStruct->IntValue = Slot.Quantity;
		const AActor* Item = Slot.Item;
		if (Item && Item->Implements<USaveableInterface>())
		{
			DataStruct->ID = ISaveableInterface::Execute_GetSaveID(Item);
		}
		OutSaveData.Add(SlotStruct);
	}
}

void UInventoryComponent::RestoreRelationships_Implementation(const TMap<FGuid, AActor*>& ActorMap)
{
	for (const auto& DataStruct : ContentToLoad)
	{
		FSlotStruct SlotStruct;
		SlotStruct.ItemID = DataStruct.NameValue;
		SlotStruct.Quantity = DataStruct.IntValue;
		if (DataStruct.ID.IsValid())
		{
			if (ActorMap.Contains(DataStruct.ID))
			{
				SlotStruct.Item = ActorMap[DataStruct.ID];
				SlotStruct.Item->SetActorTickEnabled(true);
			}
		}
		AddToInventory(SlotStruct.ItemID, SlotStruct.Quantity, SlotStruct.Item);
	}
	ContentToLoad.Empty();
}
