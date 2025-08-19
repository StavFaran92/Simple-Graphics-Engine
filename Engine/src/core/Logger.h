#pragma once
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

#pragma once
#include <memory>
#include <string>
#include <functional>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "core/Core.h"

class CallbackSink_mt; // Forward declaration

class EngineAPI Logger
{
public:
    // Initializes logger. If filePath is empty, logs to console only.
    static void init(const std::string& filePath = "");

    static std::shared_ptr<spdlog::logger>& get()
    {
        return s_logger;
    }

    // Register a callback invoked for each log message
    static void setCallback(std::function<void(spdlog::level::level_enum, const std::string&)> cb);

private:
    static void invokeCallback(spdlog::level::level_enum level, const std::string& msg);

    static std::shared_ptr<spdlog::logger> s_logger;
    static std::function<void(spdlog::level::level_enum, const std::string&)> s_callback;

    friend class CallbackSink_mt;
};

// Macros
#define logTrace(...)   Logger::get()->trace(__VA_ARGS__);
#define logInfo(...)    Logger::get()->info(__VA_ARGS__);
#define logDebug(...)   Logger::get()->debug(__VA_ARGS__)
#define logWarning(...) Logger::get()->warn(__VA_ARGS__)
#define logError(...)   Logger::get()->error(__VA_ARGS__)
