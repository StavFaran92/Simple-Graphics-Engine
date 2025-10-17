#include "core/Logger.h"
#include <spdlog/sinks/base_sink.h>

std::shared_ptr<spdlog::logger> Logger::s_logger;
std::function<void(spdlog::level::level_enum, const std::string&)> Logger::s_callback;

constexpr char* logPattern = "[%H:%M:%S] [%^%l%$] [%s:%#:%!] %v";

// Custom sink that forwards log messages to the registered callback
class CallbackSink_mt : public spdlog::sinks::base_sink<std::mutex>
{
public:
    CallbackSink_mt()
    {
        set_pattern(logPattern);
    }
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        this->formatter_->format(msg, formatted);
        Logger::invokeCallback(msg.level, std::string(formatted.begin(), formatted.end()));
    }

    void flush_() override {}
};

void Logger::init(const std::string& filePath)
{
    std::vector<spdlog::sink_ptr> sinks;

    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern(logPattern);
    sinks.push_back(consoleSink);

    if (!filePath.empty())
    {
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, true);
        fileSink->set_pattern(logPattern);
        sinks.push_back(fileSink);
    }

    // Sink to forward messages to callback
    sinks.push_back(std::make_shared<CallbackSink_mt>());

    s_logger = std::make_shared<spdlog::logger>("Engine", sinks.begin(), sinks.end());
    s_logger->set_level(spdlog::level::debug);
    spdlog::register_logger(s_logger);
}

void Logger::setCallback(std::function<void(spdlog::level::level_enum, const std::string&)> cb)
{
    s_callback = std::move(cb);
}

void Logger::invokeCallback(spdlog::level::level_enum level, const std::string& msg)
{
    if (s_callback)
    {
        s_callback(level, msg);
    }
}

