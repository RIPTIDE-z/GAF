#include "GAFMMModule.h"

IMPLEMENT_MODULE(FGAFMMModule, GAFMM)

#define LOCTEXT_NAMESPACE "GAFMMModule"

void FGAFMMModule::StartupModule()
{
	FDefaultModuleImpl::StartupModule();
}

void FGAFMMModule::ShutdownModule()
{
	FDefaultModuleImpl::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE
