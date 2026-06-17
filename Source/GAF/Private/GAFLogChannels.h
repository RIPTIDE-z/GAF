#pragma once

#include "Logging/LogMacros.h"

class UObject;

// 使用自定义Log频道，方便快速分类日志
GAF_API DECLARE_LOG_CATEGORY_EXTERN(LogGAFInput, Log, All);

GAF_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
