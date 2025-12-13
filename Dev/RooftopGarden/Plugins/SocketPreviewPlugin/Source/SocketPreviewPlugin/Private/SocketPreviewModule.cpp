#include "SocketPreviewModule.h"
#include "PreviewAttachTool.h"

#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

#define LOCTEXT_NAMESPACE "FSocketPreviewModule"

static const FName GHandleEditorTabId(TEXT("SocketPreview_HandleEditor"));

void FSocketPreviewModule::StartupModule()
{
	// Register dockable tab for the Handle Socket editor
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GHandleEditorTabId,
		FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) -> TSharedRef<SDockTab>
		{
			return SNew(SDockTab)
				.TabRole(ETabRole::PanelTab)
				.Label(LOCTEXT("HandleEditorTabLabel", "Handle Socket Editor"))
				[
					FPreviewAttachTool::CreateHandleSocketEditorWidget()
				];
		}))
		.SetDisplayName(LOCTEXT("HandleEditorTabName", "Handle Socket Editor"))
		.SetTooltipText(LOCTEXT("HandleEditorTabTooltip", "Edit the Handle socket with live preview"));

	RegisterPersonaToolbarExtension();
}

void FSocketPreviewModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GHandleEditorTabId);
	UnregisterPersonaToolbarExtension();
}

void FSocketPreviewModule::RegisterPersonaToolbarExtension()
{
	// Use ToolMenus to extend the Skeletal Mesh/Skeleton editors' toolbars in Persona
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		UToolMenus* ToolMenus = UToolMenus::Get();
		if (!ToolMenus) { return; }

		// Skeletal Mesh editor toolbar
  if (UToolMenu* Toolbar = ToolMenus->ExtendMenu("AssetEditor.SkeletalMeshEditor.ToolBar"))
  {
      FToolMenuSection& Section = Toolbar->AddSection("SocketTools", LOCTEXT("SocketToolsSection", "SocketTools"));
      Section.AddEntry(FToolMenuEntry::InitToolBarButton(
          "AddPreviewStaticMesh_Custom",
          FUIAction(FExecuteAction::CreateStatic(&FPreviewAttachTool::OnOpenAssetPickerForCurrentPersona)),
          LOCTEXT("AddPreviewStaticMesh_Label", "Add Preview StaticMesh (Custom)"),
          LOCTEXT("AddPreviewStaticMesh_Tooltip", "Add a preview StaticMesh attached to the selected socket using custom offset logic."),
          FSlateIcon()
      ));
      Section.AddEntry(FToolMenuEntry::InitToolBarButton(
          "RemovePreviewStaticMeshes_Custom",
          FUIAction(FExecuteAction::CreateStatic(&FPreviewAttachTool::RemoveAllPreviewMeshes)),
          LOCTEXT("RemovePreviewStaticMeshes_Label", "Remove Preview Meshes"),
          LOCTEXT("RemovePreviewStaticMeshes_Tooltip", "Remove all preview static meshes spawned by the plugin."),
          FSlateIcon()
      ));
  }

  // Skeleton editor toolbar (when opening a Skeleton asset)
  if (UToolMenu* Toolbar = ToolMenus->ExtendMenu("AssetEditor.SkeletonEditor.ToolBar"))
  {
      FToolMenuSection& Section = Toolbar->AddSection("SocketTools", LOCTEXT("SocketToolsSection_Skeleton", "SocketTools"));
      Section.AddEntry(FToolMenuEntry::InitToolBarButton(
          "AddPreviewStaticMesh_Custom",
          FUIAction(FExecuteAction::CreateStatic(&FPreviewAttachTool::OnOpenAssetPickerForCurrentPersona)),
          LOCTEXT("AddPreviewStaticMesh_Label", "Add Preview StaticMesh (Custom)"),
          LOCTEXT("AddPreviewStaticMesh_Tooltip", "Add a preview StaticMesh attached to the selected socket using custom offset logic."),
          FSlateIcon()
      ));
      Section.AddEntry(FToolMenuEntry::InitToolBarButton(
          "RemovePreviewStaticMeshes_Custom",
          FUIAction(FExecuteAction::CreateStatic(&FPreviewAttachTool::RemoveAllPreviewMeshes)),
          LOCTEXT("RemovePreviewStaticMeshes_Label", "Remove Preview Meshes"),
          LOCTEXT("RemovePreviewStaticMeshes_Tooltip", "Remove all preview static meshes spawned by the plugin."),
          FSlateIcon()
      ));
  }
	}));
}

void FSocketPreviewModule::UnregisterPersonaToolbarExtension()
{
	if (UToolMenus* Menus = UToolMenus::Get())
	{
		Menus->UnregisterOwner(this);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSocketPreviewModule, SocketPreviewPlugin)
