#pragma once

#include <exception>
#include <functional>
#include <string>

#include "core/Core.h"

// Crash reports: on a crash, crash/last_crash.txt next to the executable gets the error, its stack and
// the app's context; the next run reads it back (previousCrash) and moves it to previous_crash.txt.
class EngineAPI ErrorHandler
{
public:
	// Extra text for the report, like the editor's console tail; runs while crashing, so it must not block
	using CrashContext = std::function<std::string()>;

	// Runs the app with crash reporting. Under a debugger it runs unguarded, so the debugger stops instead.
	static int run(const std::function<int()>& body);

	// An exception escaped the main loop: logs and saves it with the stack where it was thrown
	static int handle(const std::exception& e, const std::string& stackTrace);

	static void setCrashContext(CrashContext context);

	// The report the previous run left; empty if it didn't crash
	static const std::string& previousCrash();
};