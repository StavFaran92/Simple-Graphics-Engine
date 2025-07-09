#pragma once
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

// Macros
#define logTrace(...) SPDLOG_TRACE(__VA_ARGS__);
#define logInfo(...) SPDLOG_INFO(__VA_ARGS__);
#define logDebug(...) SPDLOG_DEBUG(__VA_ARGS__);
#define logWarning(...) SPDLOG_WARN(__VA_ARGS__);
#define logError(...) SPDLOG_ERROR(__VA_ARGS__);