#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>

#include "MainThreadQueue.h"

namespace httplib { class Server; }

// Thrown by a tool for errors the agent should read and act on ("no entity with id 42")
struct McpToolError : std::runtime_error
{
	using std::runtime_error::runtime_error;
};

// One MCP tool. The description and input schema are all the agent knows about it, so they explain
// what the tool returns and how its arguments are used.
struct McpTool
{
	std::string name;
	std::string description;
	nlohmann::json inputSchema;

	// Runs on the editor main thread; returns an MCP content array (see mcpText / mcpImage)
	std::function<nlohmann::json(const nlohmann::json& args)> run;

	// Runs on the HTTP thread instead, while frames keep running (e.g. wait); must not touch the engine or the scene
	bool onServerThread = false;
};

inline nlohmann::json mcpText(const std::string& text)
{
	return nlohmann::json::array({ { {"type", "text"}, {"text", text} } });
}

// Base64-encodes the PNG for the JSON result (defined in McpServer.cpp, which has httplib's encoder)
nlohmann::json mcpImage(const std::vector<uint8_t>& png);

// Minimal MCP server for the editor: Streamable HTTP transport with plain JSON responses, localhost only.
// Requests arrive on server threads; every tool runs on the main thread through queue().
class McpServer
{
public:
	McpServer();
	~McpServer();

	// Register every tool before start(); the tool list is read from server threads without locking
	void addTool(McpTool tool);

	bool start(int port);
	void stop();

	MainThreadQueue& queue() { return m_queue; }

private:
	nlohmann::json handleRpc(const nlohmann::json& request);
	nlohmann::json callTool(const nlohmann::json& params);

	std::unique_ptr<httplib::Server> m_server;
	std::thread m_thread;
	std::vector<McpTool> m_tools;
	MainThreadQueue m_queue;
};
