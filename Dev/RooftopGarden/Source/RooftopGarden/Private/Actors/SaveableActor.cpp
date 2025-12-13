#include "Actors/SaveableActor.h"
#include "Interfaces/SaveableComponentInterface.h"

ASaveableActor::ASaveableActor()
{
	if (!IsTemplate() && !SaveID.IsValid())
	{
		SaveID = FGuid::NewGuid();
	}
}

FGuid ASaveableActor::GetSaveID_Implementation() const
{
	return SaveID;
}

void ASaveableActor::SetSaveID_Implementation(const FGuid& NewID)
{
	SaveID = NewID;
}

void ASaveableActor::PostEditImport()
{
	if (!IsTemplate() && SaveID.IsValid())
	{
		SaveID = FGuid::NewGuid();
		auto Components = GetComponentsByInterface(USaveableComponentInterface::StaticClass());
		for (auto& Component : Components)
		{
			ISaveableComponentInterface::Execute_SetComponentSaveID(Component, FGuid::NewGuid());
		}
	}

	Super::PostEditImport();
}
