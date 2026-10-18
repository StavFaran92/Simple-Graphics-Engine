#pragma once

// The editor's AI agent panel. For now it only connects Claude Code to the editor's MCP server
// (a one-time `claude mcp add`); the chat comes later.
class AgentWindow
{
public:
	static void display();
};
