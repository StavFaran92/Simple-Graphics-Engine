#pragma once

#include <fstream>
#include <mutex>
#include <string>

#include <nlohmann/json.hpp>

#include "core/Core.h"

/** Append-only JSON Lines next to Logger output: logsRoot/log/trace_<timestamp>.jsonl */
class EngineAPI TraceLogger
{
public:
	// Creates logsRoot/log/ if needed, opens trace_<YYYY-MM-DD_HH-MM-SS_mmm>.jsonl (append).
	// If logsRoot is empty, uses the current working directory (same base as Logger::init("test.log")).
	static void init(const std::string& logsRoot = "");

	static void writeJsonLine(const std::string& jsonLine);
	static void writeJsonLine(const nlohmann::json& j);

	static void shutdown();
	static bool isJsonlOpen();

private:
	static void closeJsonl();

	static std::mutex s_jsonlMutex;
	static std::ofstream s_jsonlFile;
};
