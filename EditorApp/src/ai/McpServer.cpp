// httplib pulls in winsock2.h, which has to come before anything that includes windows.h
#include "httplib.h"

#include "McpServer.h"

#include "core/Logger.h"

#include <algorithm>
#include <chrono>

using json = nlohmann::json;

namespace
{
	// Newest MCP revision this server follows; a client asking for another revision gets its own echoed back
	constexpr const char* kProtocolVersion = "2025-06-18";

	// Long enough for a screenshot of a big view, short enough to notice a main thread stuck in a dialog
	constexpr auto kToolTimeout = std::chrono::seconds(30);

	json rpcResult(const json& id, json result)
	{
		return { {"jsonrpc", "2.0"}, {"id", id}, {"result", std::move(result)} };
	}

	json rpcError(const json& id, int code, const std::string& message)
	{
		return { {"jsonrpc", "2.0"}, {"id", id}, {"error", { {"code", code}, {"message", message} }} };
	}

	json toolError(const std::string& message)
	{
		return { {"content", mcpText(message)}, {"isError", true} };
	}

	bool isLocalOrigin(const std::string& origin)
	{
		return origin.rfind("http://localhost", 0) == 0 || origin.rfind("http://127.0.0.1", 0) == 0;
	}
}

json mcpImage(const std::vector<uint8_t>& png)
{
	// JSON can't carry raw bytes, so MCP image content is base64 text
	const std::string data = httplib::detail::base64_encode(std::string(png.begin(), png.end()));
	return json::array({ { {"type", "image"}, {"mimeType", "image/png"}, {"data", data} } });
}

McpServer::McpServer() = default;

McpServer::~McpServer()
{
	stop();
}

void McpServer::addTool(McpTool tool)
{
	m_tools.push_back(std::move(tool));
}

bool McpServer::start(int port)
{
	m_server = std::make_unique<httplib::Server>();

	m_server->Post("/mcp", [this](const httplib::Request& req, httplib::Response& res) {
		// Browsers always send Origin; refusing foreign ones stops web pages from driving the editor (DNS rebinding)
		if (req.has_header("Origin") && !isLocalOrigin(req.get_header_value("Origin")))
		{
			res.status = 403;
			return;
		}

		json request = json::parse(req.body, nullptr, false);
		if (request.is_discarded() || !request.is_object())
		{
			res.status = 400;
			res.set_content(rpcError(nullptr, -32700, "Parse error: expected one JSON-RPC object").dump(), "application/json");
			return;
		}

		// Notifications (no id) get no response body
		if (!request.contains("id"))
		{
			res.status = 202;
			return;
		}

		res.set_content(handleRpc(request).dump(), "application/json");
	});

	// No server-initiated messages, so there is no SSE stream to open
	m_server->Get("/mcp", [](const httplib::Request&, httplib::Response& res) {
		res.status = 405;
	});

	if (!m_server->bind_to_port("127.0.0.1", port))
	{
		logWarning("MCP server: port " + std::to_string(port) + " is in use, AI tools are disabled for this session");
		m_server.reset();
		return false;
	}

	m_thread = std::thread([this] { m_server->listen_after_bind(); });

	logInfo("MCP server listening on http://127.0.0.1:" + std::to_string(port) + "/mcp");
	return true;
}

void McpServer::stop()
{
	// Release request threads waiting on the main thread before joining them
	m_queue.close();

	if (m_server)
		m_server->stop();

	if (m_thread.joinable())
		m_thread.join();

	m_server.reset();
}

json McpServer::handleRpc(const json& request)
{
	const json id = request["id"];
	const std::string method = request.value("method", "");
	const json params = request.value("params", json::object());

	if (method == "initialize")
	{
		return rpcResult(id, {
			{"protocolVersion", params.value("protocolVersion", kProtocolVersion)},
			{"capabilities", { {"tools", json::object()} }},
			{"serverInfo", { {"name", "sge-editor"}, {"version", "0.1.0"} }},
			{"instructions", "Live connection to the Simple Graphics Engine editor. Entities are addressed by "
				"their numeric id from get_scene_hierarchy. Use screenshot_viewport to see the scene."}
		});
	}

	if (method == "ping")
		return rpcResult(id, json::object());

	if (method == "tools/list")
	{
		json tools = json::array();
		for (const auto& tool : m_tools)
			tools.push_back({ {"name", tool.name}, {"description", tool.description}, {"inputSchema", tool.inputSchema} });

		return rpcResult(id, { {"tools", tools} });
	}

	if (method == "tools/call")
		return rpcResult(id, callTool(params));

	return rpcError(id, -32601, "Method not found: " + method);
}

json McpServer::callTool(const json& params)
{
	const std::string name = params.value("name", "");
	auto it = std::find_if(m_tools.begin(), m_tools.end(), [&](const McpTool& t) { return t.name == name; });
	if (it == m_tools.end())
		return toolError("Unknown tool '" + name + "'. Call tools/list for the available tools.");

	const McpTool& tool = *it;
	const json args = params.value("arguments", json::object());

	// Logged before it runs, so a call that crashes the editor is in the console tail of the crash report
	constexpr size_t kMaxLoggedArgs = 300;
	std::string loggedArgs = args.dump();
	if (loggedArgs.size() > kMaxLoggedArgs)
		loggedArgs = loggedArgs.substr(0, kMaxLoggedArgs) + "...";
	logInfo("[MCP] {} {}", name, loggedArgs);

	auto future = m_queue.post([&tool, args] { return tool.run(args); });
	if (future.wait_for(kToolTimeout) != std::future_status::ready)
		return toolError("Timed out waiting for the editor main thread. It may be blocked by a modal dialog or a breakpoint.");

	try
	{
		return { {"content", future.get()} };
	}
	catch (const McpToolError& e)
	{
		return toolError(e.what());
	}
	catch (const std::future_error&)
	{
		return toolError("The editor is shutting down.");
	}
	catch (const std::exception& e)
	{
		return toolError(std::string("Tool '") + name + "' failed: " + e.what());
	}
}
