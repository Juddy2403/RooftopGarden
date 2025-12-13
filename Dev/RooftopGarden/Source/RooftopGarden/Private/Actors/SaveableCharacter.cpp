// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/SaveableCharacter.h"

#include "Interfaces/SaveableComponentInterface.h"

// Sets default values
ASaveableCharacter::ASaveableCharacter()
{

}

void ASaveableCharacter::PostInitializeComponents()
{
	// Giving the player's components ids
	auto PlayerComponents = GetComponentsByInterface(USaveableComponentInterface::StaticClass());
	for (auto& Component : PlayerComponents)
	{
		FGuid id = ISaveableComponentInterface::Execute_GetComponentSaveID(Component);
		if (id.IsValid()) continue;
		ISaveableComponentInterface::Execute_SetComponentSaveID(Component, FGuid::NewGuid());
	}
	Super::PostInitializeComponents();
}





