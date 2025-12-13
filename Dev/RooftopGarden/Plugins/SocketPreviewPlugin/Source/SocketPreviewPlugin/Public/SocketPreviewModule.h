#pragma once

#include "Modules/ModuleManager.h"

class FSocketPreviewModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterPersonaToolbarExtension();
	void UnregisterPersonaToolbarExtension();

	TSharedPtr<class FExtensibilityManager> PersonaToolbarExtensibilityManager;
};
