#pragma once

#include <functional>
#include <string>
#include <vector>

class McpServer;

struct ConsoleLine
{
	int level; // spdlog::level::level_enum
	std::string text;
};

// The console log lives in Editor.cpp; tools read it through this callback
using ConsoleReader = std::function<std::vector<ConsoleLine>()>;

// Registers the read-only MCP tools an AI agent uses to inspect the editor
// (scene, entities, assets, console, screenshots). Not related to EditorTool, the viewport tools.
void registerAgentTools(McpServer& server, ConsoleReader readConsole);
