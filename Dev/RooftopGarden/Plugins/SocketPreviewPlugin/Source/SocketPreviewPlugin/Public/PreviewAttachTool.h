#pragma once

#include "CoreMinimal.h"

class SWindow;
class SWidget;

class FPreviewAttachTool
{
public:
	// Called by toolbar button: finds current Persona preview and opens an asset picker
	static void OnOpenAssetPickerForCurrentPersona();

	// Called by toolbar button: removes all spawned preview static mesh components
	static void RemoveAllPreviewMeshes();

	// Provides content for the dockable Handle Socket Editor tab
	static TSharedRef<SWidget> CreateHandleSocketEditorWidget();

private:
	// Called when asset is selected from the picker
	static void OnStaticMeshPicked(const FAssetData& AssetData, TWeakObjectPtr<class USkeletalMeshComponent> PersonaPreviewMeshComp, FName SelectedSocketName);

	// Helper: find the "active" Persona preview skeletal mesh component
	static USkeletalMeshComponent* GetActivePersonaPreviewMesh();
};
