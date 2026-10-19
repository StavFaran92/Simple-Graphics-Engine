#include "core/ErrorHandler.h"

#include "core/Logger.h"

#include <cpptrace/cpptrace.hpp>
#include <cpptrace/from_current.hpp>

#define NOMINMAX
#include <Windows.h>

#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace
{
	fs::path s_directory;
	std::string s_previousCrash;
	ErrorHandler::CrashContext s_crashContext;

	void saveReport(const std::string& report)
	{
		char time[32];
		const std::time_t now = std::time(nullptr);
		std::tm local{};
		localtime_s(&local, &now);
		std::strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", &local);

		std::ofstream out(s_directory / "last_crash.txt");
		out << "Crashed at " << time << "\n" << report << std::flush;

		// Last: it reads app state, which may be what broke
		if (s_crashContext)
		{
			try
			{
				out << "\n" << s_crashContext();
			}
			catch (...)
			{
				out << "\n(context unavailable)\n";
			}
		}
	}

	// Crashes the try/catch in run() can't catch, like an access violation
	LONG WINAPI onUnhandledException(EXCEPTION_POINTERS* pointers)
	{
		std::ostringstream report;
		report << "Exception 0x" << std::hex << pointers->ExceptionRecord->ExceptionCode << std::dec
			<< "\n\nStack:\n" << cpptrace::generate_trace().to_string();
		saveReport(report.str());
		TerminateProcess(GetCurrentProcess(), 1);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	// For example an exception thrown out of a noexcept function
	void onTerminate()
	{
		saveReport("std::terminate called\n\nStack:\n" + cpptrace::generate_trace().to_string());
		TerminateProcess(GetCurrentProcess(), 1);
	}

	void installCrashReporting()
	{
		wchar_t exePath[MAX_PATH];
		GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		s_directory = fs::path(exePath).parent_path() / "crash";

		std::error_code ec;
		fs::create_directories(s_directory, ec);

		// Hand the last run's report over once, then move it aside so it isn't reported twice
		const fs::path last = s_directory / "last_crash.txt";
		if (fs::exists(last, ec))
		{
			{
				std::ifstream in(last);
				std::stringstream text;
				text << in.rdbuf();
				s_previousCrash = text.str();
			}
			fs::rename(last, s_directory / "previous_crash.txt", ec);
		}

		SetUnhandledExceptionFilter(onUnhandledException);
		std::set_terminate(onTerminate);

		// No "abort() has been called" dialog: it would hang the app instead of ending it
		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
	}
}

int ErrorHandler::run(const std::function<int()>& body)
{
	if (IsDebuggerPresent())
		return body();

	installCrashReporting();

	// The try body is a lambda on Windows, so it can't 'return'
	int result = 1;
	CPPTRACE_TRY
	{
		result = body();
	}
	CPPTRACE_CATCH(const std::exception& e)
	{
		result = handle(e, cpptrace::from_current_exception().to_string());
	}
	return result;
}

int ErrorHandler::handle(const std::exception& e, const std::string& stackTrace)
{
	// One line in the log; the stack goes to the report file, which also gets the log's tail
	logError("Uncaught exception: {} (report: {})", e.what(), (s_directory / "last_crash.txt").string());
	Logger::get()->flush();

	saveReport("Uncaught exception: " + std::string(e.what()) + "\n\nStack where it was thrown:\n" + stackTrace);
	return 1;
}

void ErrorHandler::setCrashContext(CrashContext context)
{
	s_crashContext = std::move(context);
}

const std::string& ErrorHandler::previousCrash()
{
	return s_previousCrash;
}
