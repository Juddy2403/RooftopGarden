
#include "Components/SaveableComponent.h"

FGuid USaveableComponent::GetComponentSaveID_Implementation() const
{
	return SaveID;
}

void USaveableComponent::SetComponentSaveID_Implementation(const FGuid& NewID)
{
	SaveID = NewID;
}
