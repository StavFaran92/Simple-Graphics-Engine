#include "core/TraceLogger.h"

#include "core/Logger.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

std::mutex TraceLogger::s_jsonlMutex;
std::ofstream TraceLogger::s_jsonlFile;

namespace
{
	std::string makeTimestampFilename()
	{
		const auto now = std::chrono::system_clock::now();
		const std::time_t t = std::chrono::system_clock::to_time_t(now);
		std::tm tm{};
		localtime_s(&tm, &t);
		char buf[32];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tm);
		const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
		std::ostringstream oss;
		oss << "trace_" << buf << '_' << std::setfill('0') << std::setw(3) << ms.count() << ".jsonl";
		return oss.str();
	}
}

void TraceLogger::closeJsonl()
{
	std::lock_guard<std::mutex> lock(s_jsonlMutex);
	if (s_jsonlFile.is_open())
	{
		s_jsonlFile.flush();
		s_jsonlFile.close();
	}
}

void TraceLogger::init(const std::string& logsRoot)
{
	closeJsonl();

	fs::path base = logsRoot.empty() ? fs::current_path() : fs::path(logsRoot);
	fs::path logsDir = base / "logs";
	std::error_code ec;
	fs::create_directories(logsDir, ec);
	if (ec)
	{
		logError("TraceLogger: failed to create logs directory: {}", logsDir.string());
		return;
	}

	const fs::path filePath = logsDir / makeTimestampFilename();

	std::lock_guard<std::mutex> lock(s_jsonlMutex);
	s_jsonlFile.open(filePath, std::ios::out | std::ios::app);
	if (!s_jsonlFile.is_open())
	{
		logError("TraceLogger: failed to open JSONL file: {}", filePath.string());
	}
}

void TraceLogger::writeJsonLine(const std::string& jsonLine)
{
	std::lock_guard<std::mutex> lock(s_jsonlMutex);
	if (!s_jsonlFile.is_open())
	{
		return;
	}

	for (char c : jsonLine)
	{
		if (c != '\n' && c != '\r')
		{
			s_jsonlFile.put(c);
		}
		else
		{
			s_jsonlFile.put(' ');
		}
	}
	s_jsonlFile.put('\n');
	s_jsonlFile.flush();
}

void TraceLogger::writeJsonLine(const nlohmann::json& j)
{
	writeJsonLine(j.dump());
}

void TraceLogger::shutdown()
{
	closeJsonl();
}

bool TraceLogger::isJsonlOpen()
{
	std::lock_guard<std::mutex> lock(s_jsonlMutex);
	return s_jsonlFile.is_open();
}
