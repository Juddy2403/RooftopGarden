// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveableComponent.h"
#include "Components/ActorComponent.h"
#include "PhysicsItemComponent.generated.h"


class UBoxComponent;
class AStaticMeshActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOFTOPGARDEN_API UPhysicsItemComponent : public USaveableComponent
{
	GENERATED_BODY()

public:	
	UPhysicsItemComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* SpawnLimitBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* SpawnAreaBox;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 TotalHeldItems = 0;

	UFUNCTION(BlueprintCallable)
	bool SpawnActorIfWithinLimit(UStaticMesh* ActorMesh);
	
	UFUNCTION(BlueprintCallable)
	bool HasReachedLimit() const;

	UFUNCTION(BlueprintCallable)
	bool DiscardLastItem();

	UFUNCTION(BlueprintCallable)
	void Reset();

private:
	UFUNCTION(BlueprintCallable)
	bool SpawnAndSetupActor(UStaticMesh* ActorMesh);
	
	UPROPERTY()
	TArray<TObjectPtr<AStaticMeshActor>> SpawnedActors;

	UFUNCTION()
	FVector GetRandomSpawnPoint() const;
	
};
