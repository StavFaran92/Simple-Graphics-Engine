#pragma once
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

#pragma once
#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "core/Core.h"

class EngineAPI Logger
{
public:
    // Initializes logger. If filePath is empty, logs to console only.
    static void init(const std::string& filePath = "")
    {
        std::vector<spdlog::sink_ptr> sinks;

        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_pattern("[%T] [%^%l%$] %v");
        sinks.push_back(consoleSink);

        if (!filePath.empty())
        {
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, true);
            fileSink->set_pattern("[%Y-%m-%d %T] [%l] %v");
            sinks.push_back(fileSink);
        }

        s_logger = std::make_shared<spdlog::logger>("Engine", sinks.begin(), sinks.end());
        s_logger->set_level(spdlog::level::trace);
        spdlog::register_logger(s_logger);
    }

    static std::shared_ptr<spdlog::logger>& get()
    {
        return s_logger;
    }

private:
    static std::shared_ptr<spdlog::logger> s_logger;
};

// Macros
#define logTrace(...)   Logger::get()->trace(__VA_ARGS__);
#define logInfo(...)    Logger::get()->info(__VA_ARGS__);
#define logDebug(...)   Logger::get()->debug(__VA_ARGS__)
#define logWarning(...) Logger::get()->warn(__VA_ARGS__)
#define logError(...)   Logger::get()->error(__VA_ARGS__)