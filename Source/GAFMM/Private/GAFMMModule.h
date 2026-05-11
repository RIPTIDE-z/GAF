#pragma once

#include "Modules/ModuleManager.h"

class FGAFMMModule : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};