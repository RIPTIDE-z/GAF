#pragma once

#include "Logging/LogMacros.h"

class UObject;

GAF_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
