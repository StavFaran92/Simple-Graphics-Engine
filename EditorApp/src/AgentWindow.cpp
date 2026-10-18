#include "AgentWindow.h"

#include "Common.h"
#include "imgui_stdlib.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <sstream>

namespace fs = std::filesystem;

namespace
{
	// Must match the server McpServer::start() opens in Editor.cpp
	constexpr const char* kServerName = "sge-editor";
	constexpr const char* kServerUrl = "http://127.0.0.1:7777/mcp";

	struct CommandResult
	{
		int exitCode = -1;
		std::string output;
	};

	// Where Claude Code installs itself: PATH, the native installer, then the CLI bundled with the VS Code extension
	std::string findClaude()
	{
		std::vector<fs::path> candidates;

		if (const char* path = std::getenv("PATH"))
		{
			std::stringstream dirs(path);
			std::string dir;
			while (std::getline(dirs, dir, ';'))
			{
				if (!dir.empty())
					candidates.push_back(fs::path(dir) / "claude.exe");
			}
		}

		if (const char* home = std::getenv("USERPROFILE"))
		{
			candidates.push_back(fs::path(home) / ".local" / "bin" / "claude.exe");

			// Several extension versions can sit side by side; take the most recently installed one
			std::error_code ec;
			fs::path newest;
			fs::file_time_type newestTime{};
			for (const auto& entry : fs::directory_iterator(fs::path(home) / ".vscode" / "extensions", ec))
			{
				if (entry.path().filename().string().rfind("anthropic.claude-code", 0) != 0)
					continue;

				const fs::path exe = entry.path() / "resources" / "native-binary" / "claude.exe";
				const auto installed = entry.last_write_time(ec);
				if (fs::exists(exe, ec) && (newest.empty() || installed > newestTime))
				{
					newest = exe;
					newestTime = installed;
				}
			}
			if (!newest.empty())
				candidates.push_back(newest);
		}

		for (const auto& candidate : candidates)
		{
			std::error_code ec;
			if (fs::exists(candidate, ec))
				return candidate.string();
		}
		return "";
	}

	// SGE_ROOT_DIR is "<repo>/Engine/". The default (local) MCP scope belongs to the folder the command runs in,
	// and the editor runs from build/EditorApp, so commands must run from the repo root.
	std::string repoRoot()
	{
		return fs::path(SGE_ROOT_DIR).parent_path().parent_path().make_preferred().string();
	}

	// Runs through cmd.exe in the editor's own console (no window pops up); returns stdout + stderr
	CommandResult run(const std::string& commandLine)
	{
		CommandResult result;

		FILE* pipe = _popen((commandLine + " 2>&1").c_str(), "r");
		if (!pipe)
		{
			result.output = "Could not run: " + commandLine;
			return result;
		}

		char buffer[512];
		while (fgets(buffer, sizeof(buffer), pipe))
			result.output += buffer;

		result.exitCode = _pclose(pipe);
		return result;
	}

	// Registers the editor's MCP server with Claude Code unless it already is (`mcp get` fails when it isn't)
	CommandResult connect(const std::string& claude, const std::string& root)
	{
		const std::string prefix = "cd /d \"" + root + "\" && \"" + claude + "\" ";

		CommandResult existing = run(prefix + "mcp get " + kServerName);
		if (existing.exitCode == 0)
		{
			existing.output = "Already registered.\n\n" + existing.output;
			return existing;
		}

		return run(prefix + "mcp add --transport http " + kServerName + " " + kServerUrl);
	}
}

void AgentWindow::display()
{
	static std::string claudePath = findClaude();
	static std::future<CommandResult> pending;
	static CommandResult lastResult;
	static bool hasResult = false;

	if (pending.valid() && pending.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		lastResult = pending.get();
		hasResult = true;
	}
	const bool connecting = pending.valid();

	ImGui::Begin("Agent", nullptr, windowFlags);

	ImGui::TextWrapped("Connect Claude Code to this editor so an agent can inspect the open scene. "
		"This registers the editor's tool server once for this project.");
	ImGui::Separator();

	ImGui::Text("Claude Code CLI");
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##claudePath", &claudePath);
	if (claudePath.empty())
		ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.3f, 1.0f), "Not found. Install Claude Code or enter the path to claude.exe.");

	ImGui::BeginDisabled(connecting || claudePath.empty());
	if (ImGui::Button(connecting ? "Connecting..." : "Connect to Claude Code"))
		pending = std::async(std::launch::async, connect, claudePath, repoRoot());
	ImGui::EndDisabled();

	if (hasResult)
	{
		ImGui::Spacing();
		if (lastResult.exitCode == 0)
		{
			ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Connected.");
			ImGui::TextWrapped("New Claude Code sessions in this project can use the editor tools. "
				"In a session that is already open, run /mcp to reconnect.");
		}
		else
		{
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Failed (exit code %d)", lastResult.exitCode);
		}

		ImGui::BeginChild("##output", ImVec2(0, 0), true);
		ImGui::TextUnformatted(lastResult.output.c_str());
		ImGui::EndChild();
	}

	ImGui::End();
}
