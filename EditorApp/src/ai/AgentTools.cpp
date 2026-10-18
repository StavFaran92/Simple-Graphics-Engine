#include "AgentTools.h"

#include "McpServer.h"
#include "ViewCapture.h"
#include "EntityState.h"

#include "sge.h"
#include "core/Logger.h"
#include "core/Configurations.h"
#include "serialize/Archiver.h"
#include "memory/Assets.h"
#include "memory/AssetRecord.h"

#include <algorithm>
#include <fstream>

using json = nlohmann::json;

namespace
{
	// Asset files above this are reported by metadata only, to keep tool results small
	constexpr size_t kMaxAssetBytes = 256 * 1024;

	json objectSchema(json properties = json::object(), json required = json::array())
	{
		json schema = { {"type", "object"}, {"properties", std::move(properties)} };
		if (!required.empty())
			schema["required"] = std::move(required);
		return schema;
	}

	// The type names list_assets reports, straight from the engine's own mapping
	json assetTypeNames()
	{
		json names = json::array();
		for (auto& [type, name] : g_assetTypeToStr)
		{
			if (type != AssetType::NONE)
				names.push_back(name);
		}
		return names;
	}

	std::string trimNewlines(const std::string& text)
	{
		const size_t end = text.find_last_not_of("\r\n");
		return end == std::string::npos ? "" : text.substr(0, end + 1);
	}

	std::vector<Entity> sceneEntities()
	{
		auto scene = Engine::get()->getContext()->getActiveScene();

		std::vector<Entity> entities;
		for (auto entity : scene->getRegistry().get().view<ObjectComponent>())
			entities.emplace_back(entity, &scene->getRegistry());

		std::sort(entities.begin(), entities.end(), [](const Entity& a, const Entity& b) {
			return a.handlerID() < b.handlerID();
		});
		return entities;
	}

	Entity entityFromArgs(const json& args)
	{
		if (!args.contains("id") || !args["id"].is_number_integer() || args["id"].get<int64_t>() < 0)
			throw McpToolError("'id' must be a numeric entity id, as listed by get_scene_hierarchy");

		const auto id = args["id"].get<uint32_t>();
		auto scene = Engine::get()->getContext()->getActiveScene();

		Entity e{ static_cast<entt::entity>(id), &scene->getRegistry() };
		if (!e.valid())
			throw McpToolError("No entity with id " + std::to_string(id) + " in the active scene");
		return e;
	}

	std::string entityName(Entity e)
	{
		return e.HasComponent<ObjectComponent>() ? e.getComponent<ObjectComponent>().name : "";
	}

	Entity parentOf(Entity e)
	{
		return e.HasComponent<Transformation>() ? e.getComponent<Transformation>().getParent() : Entity::EmptyEntity;
	}

	std::vector<Entity> childrenOf(Entity e)
	{
		std::vector<Entity> children;
		if (!e.HasComponent<Transformation>())
			return children;

		for (auto& [_, child] : e.getComponent<Transformation>().getChildren())
			children.push_back(child);

		std::sort(children.begin(), children.end(), [](const Entity& a, const Entity& b) {
			return a.handlerID() < b.handlerID();
		});
		return children;
	}

	// { "Transformation": { ...fields }, "MeshRendererComponent": { ... } } in the same form the .scene file stores
	json componentsOf(Entity e)
	{
		json archive = json::parse(Archiver::serializeEntityToJson(e));

		json components = json::object();
		for (const auto& c : archive["entity"]["components"])
		{
			// cereal wraps polymorphic pointers as { polymorphic_name, ptr_wrapper: { data } }
			if (c.contains("polymorphic_name") && c.contains("ptr_wrapper"))
				components[c["polymorphic_name"].get<std::string>()] = c["ptr_wrapper"].value("data", json::object());
		}
		return components;
	}

	json hierarchyNode(Entity e, int depth, int maxDepth, bool withComponents)
	{
		json node = { {"id", e.handlerID()}, {"name", entityName(e)} };

		if (withComponents)
		{
			// Held in a local: items() only references the object, it must outlive the loop
			const json components = componentsOf(e);

			json names = json::array();
			for (auto& item : components.items())
				names.push_back(item.key());
			node["components"] = names;
		}

		auto children = childrenOf(e);
		if (children.empty())
			return node;

		if (depth >= maxDepth)
		{
			node["childCount"] = children.size();
			return node;
		}

		json childNodes = json::array();
		for (auto& child : children)
			childNodes.push_back(hierarchyNode(child, depth + 1, maxDepth, withComponents));
		node["children"] = childNodes;
		return node;
	}

	json assetSummary(const AssetRecord& record)
	{
		return {
			{"name", record.name},
			{"uuid", record.uuid.str()},
			{"type", getAssetTypeAsStr(record.aType)},
			{"directory", record.assetDirectory},
			{"file", record.getFilename()},
		};
	}
}

void registerAgentTools(McpServer& server, ConsoleReader readConsole)
{
	// The tree of entities: id + name + component names + children
	server.addTool({
		"get_scene_hierarchy",
		"Lists the entities of the active scene as a tree. Each node has the entity id (use it in other tools), "
		"its name, its component type names, and its children. Nodes deeper than max_depth report childCount instead.",
		objectSchema({
			{"max_depth", { {"type", "integer"}, {"description", "Levels of children to expand (default 16)"} }},
			{"include_components", { {"type", "boolean"}, {"description", "List component names per entity (default true)"} }},
		}),
		[](const json& args) {
			const int maxDepth = args.value("max_depth", 16);
			const bool withComponents = args.value("include_components", true);

			const auto entities = sceneEntities();

			json roots = json::array();
			for (auto& e : entities)
			{
				if (!parentOf(e).valid())
					roots.push_back(hierarchyNode(e, 0, maxDepth, withComponents));
			}

			return mcpText(json{ {"entityCount", entities.size()}, {"entities", roots} }.dump());
		}
	});

	// One entity in full: what the Inspector shows
	server.addTool({
		"get_entity",
		"Returns one entity with every component and its current field values, in the same JSON form the "
		".scene file stores (asset references appear as {\"uuid\": {\"m_value\": N}}). Also returns parent and child ids.",
		objectSchema({
			{"id", { {"type", "integer"}, {"description", "Entity id from get_scene_hierarchy"} }},
		}, { "id" }),
		[](const json& args) {
			Entity e = entityFromArgs(args);

			json children = json::array();
			for (auto& child : childrenOf(e))
				children.push_back(child.handlerID());

			Entity parent = parentOf(e);

			return mcpText(json{
				{"id", e.handlerID()},
				{"name", entityName(e)},
				{"parent", parent.valid() ? json(parent.handlerID()) : json(nullptr)},
				{"children", children},
				{"components", componentsOf(e)},
			}.dump());
		}
	});

	// Which components exist and what their fields look like, so the agent doesn't guess field names
	server.addTool({
		"get_component_schema",
		"Lists every registered component type, plus an example of each component's fields taken from the first "
		"entity in the active scene that has it. Components with no example are registered but unused in this scene.",
		objectSchema(),
		[](const json&) {
			json examples = json::object();
			for (auto& e : sceneEntities())
			{
				const json components = componentsOf(e);
				for (auto& item : components.items())
				{
					if (!examples.contains(item.key()))
						examples[item.key()] = item.value();
				}
			}

			return mcpText(json{
				{"registered", Archiver::getRegisteredComponentNames()},
				{"examplesFromScene", examples},
			}.dump());
		}
	});

	// The Asset View as a list
	server.addTool({
		"list_assets",
		"Lists project assets: name, uuid, type, directory and file. Engine built-in assets are hidden unless include_engine is true.",
		objectSchema({
			{"type", { {"type", "string"}, {"enum", assetTypeNames()} }},
			{"folder", { {"type", "string"}, {"description", "Only assets whose directory contains this text"} }},
			{"include_engine", { {"type", "boolean"}, {"description", "Include engine built-in assets (default false)"} }},
			{"limit", { {"type", "integer"}, {"description", "Maximum assets returned (default 200)"} }},
		}),
		[](const json& args) {
			const std::string typeFilter = args.value("type", "");
			const std::string folderFilter = args.value("folder", "");
			const bool includeEngine = args.value("include_engine", false);
			const size_t limit = args.value("limit", 200);

			json assets = json::array();
			size_t matched = 0;
			for (const AssetRecord* record : Engine::get()->getSubSystem<Assets>()->getAllRecords())
			{
				if (!record || (record->isEngineOwned && !includeEngine))
					continue;
				if (!typeFilter.empty() && getAssetTypeAsStr(record->aType) != typeFilter)
					continue;
				if (!folderFilter.empty() && record->assetDirectory.find(folderFilter) == std::string::npos)
					continue;

				if (++matched <= limit)
					assets.push_back(assetSummary(*record));
			}

			return mcpText(json{ {"matched", matched}, {"returned", assets.size()}, {"assets", assets} }.dump());
		}
	});

	// One asset's metadata plus its file content when it's small JSON (materials, prefabs)
	server.addTool({
		"get_asset",
		"Returns one asset's metadata and, for text assets such as materials and prefabs, its file content as JSON. "
		"Binary assets and files over 256 KB return metadata only. Pass uuid or name.",
		objectSchema({
			{"uuid", { {"type", "string"}, {"description", "Asset uuid from list_assets"} }},
			{"name", { {"type", "string"}, {"description", "Asset name, used when uuid is not given"} }},
		}),
		[](const json& args) {
			std::string uuid;
			if (args.contains("uuid"))
				uuid = args["uuid"].is_string() ? args["uuid"].get<std::string>() : args["uuid"].dump();
			const std::string name = args.value("name", "");

			if (uuid.empty() && name.empty())
				throw McpToolError("Pass 'uuid' or 'name' of an asset from list_assets");

			const AssetRecord* found = nullptr;
			for (const AssetRecord* record : Engine::get()->getSubSystem<Assets>()->getAllRecords())
			{
				if (record && (uuid.empty() ? record->name == name : record->uuid.str() == uuid))
				{
					found = record;
					break;
				}
			}
			if (!found)
				throw McpToolError("No asset with " + (uuid.empty() ? "name '" + name + "'" : "uuid " + uuid) + ". Use list_assets to find one.");

			json result = assetSummary(*found);
			result["path"] = found->getAbsolutePath();

			std::ifstream file(found->getAbsolutePath(), std::ios::binary);
			std::string bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

			if (bytes.size() > kMaxAssetBytes)
			{
				result["note"] = "File is " + std::to_string(bytes.size() / 1024) + " KB; content not included";
			}
			else
			{
				json content = json::parse(bytes, nullptr, false);
				if (content.is_discarded())
					result["note"] = "Binary file; content not included";
				else
					result["content"] = content;
			}

			return mcpText(result.dump());
		}
	});

	// The Console window's last lines
	server.addTool({
		"get_console",
		"Returns the most recent editor console lines, oldest first, at or above min_level. "
		"Each line reads [time] [level] [file:line] message.",
		objectSchema({
			{"lines", { {"type", "integer"}, {"description", "How many lines (default 50, max 500)"} }},
			{"min_level", { {"type", "string"}, {"enum", { "trace", "debug", "info", "warning", "error", "critical" }} }},
		}),
		[readConsole](const json& args) {
			const size_t maxLines = std::clamp(args.value("lines", 50), 1, 500);
			const auto minLevel = spdlog::level::from_str(args.value("min_level", "info"));

			const auto all = readConsole();

			std::vector<std::string> picked;
			for (auto it = all.rbegin(); it != all.rend() && picked.size() < maxLines; ++it)
			{
				if (it->level >= minLevel)
					picked.push_back(trimNewlines(it->text));
			}
			std::reverse(picked.begin(), picked.end());

			std::string text;
			for (auto& line : picked)
				text += line + "\n";

			return mcpText(text.empty() ? "(no matching lines)" : text);
		}
	});

	// What the viewport shows, as an image the agent can look at
	server.addTool({
		"screenshot_viewport",
		"Captures what a viewport currently shows as a PNG image. 'editor' is the scene view seen through the "
		"editor camera; 'game' is the game camera's view and may only exist while the simulation runs.",
		objectSchema({
			{"view", { {"type", "string"}, {"enum", { "editor", "game" }}, {"description", "Default editor"} }},
			{"max_width", { {"type", "integer"}, {"description", "Downscale to at most this width in pixels (default 1024, 0 = full size)"} }},
		}),
		[](const json& args) {
			const std::string view = args.value("view", "editor");
			if (view != "editor" && view != "game")
				throw McpToolError("'view' must be \"editor\" or \"game\"");

			const std::string viewName = view == "game" ? "Game View" : "Editor View";

			CapturedImage image;
			if (!captureRenderView(viewName, image))
				throw McpToolError("The '" + viewName + "' render view is not available" +
					(view == "game" ? " - start the simulation first" : ""));

			CapturedImage scaled = downscaleToWidth(image, args.value("max_width", 1024));

			std::string caption = viewName + ", " + std::to_string(scaled.width) + "x" + std::to_string(scaled.height);
			if (scaled.width != image.width)
				caption += " (downscaled from " + std::to_string(image.width) + "x" + std::to_string(image.height) + ")";

			json content = mcpImage(encodePng(scaled));
			content.push_back({ {"type", "text"}, {"text", caption} });
			return content;
		}
	});

	// What the user has selected, so "fix this one" has a target
	server.addTool({
		"get_selection",
		"Returns the entity currently selected in the editor (id and name), or null when nothing is selected.",
		objectSchema(),
		[](const json&) {
			Entity e = state.getSelectedEntity();
			if (!e.valid())
				return mcpText(json{ {"selected", nullptr} }.dump());

			return mcpText(json{ {"selected", { {"id", e.handlerID()}, {"name", entityName(e)} }} }.dump());
		}
	});
}
