#include "AgentTools.h"

#include "McpServer.h"
#include "ViewCapture.h"
#include "EntityState.h"
#include "Common.h"
#include "EntityPresets.h"
#include "EditorCamera.h"

#include "sge.h"
#include "core/Logger.h"
#include "core/Configurations.h"
#include "core/ErrorHandler.h"
#include "serialize/Archiver.h"
#include "memory/Assets.h"
#include "memory/AssetRecord.h"
#include "runtime/StartupSceneTemplate.h"

#include <algorithm>
#include <fstream>
#include <optional>

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

	Entity entityFromArgs(const json& args, const std::string& field = "id")
	{
		// Above 32 bits it would be truncated to another entity's id (2^40 -> 0)
		if (!args.contains(field) || !args[field].is_number_integer() ||
			args[field].get<int64_t>() < 0 || args[field].get<int64_t>() > UINT32_MAX)
			throw McpToolError("'" + field + "' must be a numeric entity id, as listed by get_scene_hierarchy");

		const auto id = args[field].get<uint32_t>();
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

	// '"Wall_N" (id 42)' for replies
	std::string describe(Entity e)
	{
		return "\"" + entityName(e) + "\" (id " + std::to_string(e.handlerID()) + ")";
	}

	size_t countDescendants(Entity e)
	{
		size_t count = 0;
		for (auto& child : childrenOf(e))
			count += 1 + countDescendants(child);
		return count;
	}

	std::string joinNames(const std::vector<std::string>& names)
	{
		std::string joined;
		for (const auto& name : names)
			joined += (joined.empty() ? "" : ", ") + name;
		return joined;
	}

	// Edits during play would be recorded and later undone into the running game
	void requireEditMode()
	{
		if (Engine::get()->getContext()->getActiveScene()->isSimulationActive())
			throw McpToolError("The simulation is running; stop it before changing the scene");
	}

	// Inverse of componentsOf's unwrapping: fields -> the cereal JSON Archiver::loadComponent reads
	std::string toCerealJson(const std::string& component, const json& fields)
	{
		// The ids cereal gives the first polymorphic pointer in an archive
		constexpr uint32_t kFirstId = 0x80000001;
		return json{ {"component", {
			{"polymorphic_id", kFirstId},
			{"polymorphic_name", component},
			{"ptr_wrapper", { {"id", kFirstId}, {"data", fields} }},
		}} }.dump();
	}

	// The component's fields as get_entity shows them, or null when the entity doesn't have it
	json componentFields(Entity e, const std::string& component)
	{
		std::string data;
		if (!Archiver::saveComponent(e, component, data))
			return nullptr;
		return json::parse(data)["component"]["ptr_wrapper"]["data"];
	}

	// The given fields over the component's current ones; a component the entity lacks needs all of them
	std::string componentWithFields(Entity e, const std::string& component, const json& fields)
	{
		json merged = componentFields(e, component);
		if (merged.is_null())
			merged = fields;
		else
			merged.merge_patch(fields);
		return toCerealJson(component, merged);
	}

	glm::vec3 vec3FromArgs(const json& args, const std::string& field)
	{
		const json& value = args[field];
		if (!value.is_array() || value.size() != 3 || !value[0].is_number() || !value[1].is_number() || !value[2].is_number())
			throw McpToolError("'" + field + "' must be [x, y, z]");
		return { value[0].get<float>(), value[1].get<float>(), value[2].get<float>() };
	}

	const AssetRecord& findAssetRecord(const json& args)
	{
		std::string uuid;
		if (args.contains("uuid"))
			uuid = args["uuid"].is_string() ? args["uuid"].get<std::string>() : args["uuid"].dump();
		const std::string name = args.value("name", "");

		if (uuid.empty() && name.empty())
			throw McpToolError("Pass 'uuid' or 'name' of an asset from list_assets");

		for (const AssetRecord* record : Engine::get()->getSubSystem<Assets>()->getAllRecords())
		{
			if (record && (uuid.empty() ? record->name == name : record->uuid.str() == uuid))
				return *record;
		}
		throw McpToolError("No asset with " + (uuid.empty() ? "name '" + name + "'" : "uuid " + uuid) + ". Use list_assets to find one.");
	}

	void runOrThrow(std::unique_ptr<Command> command)
	{
		std::string error;
		if (!CommandStack::Instance().run(std::move(command), error))
			throw McpToolError(error);
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
		"Without 'component': lists every registered component type. With 'component': its fields as a "
		"default-constructed instance ('defaults'), plus 'exampleFromScene' from the first entity that has it. "
		"Defaults show field names and types but may not be a useful setup (e.g. a physics collider is null); "
		"prefer create_preset for things the Create menu offers.",
		objectSchema({
			{"component", { {"type", "string"}, {"description", "A name from the registered list"} }},
		}),
		[](const json& args) {
			const auto registered = Archiver::getRegisteredComponentNames();
			const std::string component = args.value("component", "");
			if (component.empty())
				return mcpText(json{ {"registered", registered} }.dump());

			if (std::find(registered.begin(), registered.end(), component) == registered.end())
				throw McpToolError("Unknown component '" + component + "'. Registered: " + joinNames(registered));

			json result = { {"component", component} };

			std::string data;
			if (Archiver::defaultComponent(component, data))
				result["defaults"] = json::parse(data)["component"]["ptr_wrapper"]["data"];
			else
			{
				result["defaults"] = nullptr;
				result["note"] = "No default constructor; copy exampleFromScene or use create_preset";
			}

			for (auto& e : sceneEntities())
			{
				const json fields = componentFields(e, component);
				if (!fields.is_null())
				{
					result["exampleFromScene"] = fields;
					result["exampleEntity"] = describe(e);
					break;
				}
			}

			return mcpText(result.dump());
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
			const AssetRecord& found = findAssetRecord(args);

			json result = assetSummary(found);
			result["path"] = found.getAbsolutePath();

			std::ifstream file(found.getAbsolutePath(), std::ios::binary);
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

	server.addTool({
		"get_last_crash",
		"If the previous editor run crashed: the error, the stack where it happened, and the console's last lines "
		"(including the [MCP] tool calls that led to it). Call it after the editor restarts from a crash.",
		objectSchema(),
		[](const json&) {
			const std::string& report = ErrorHandler::previousCrash();
			return mcpText(report.empty() ? "The previous run didn't crash." : report);
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

	// ---- Editor camera: moves the view, not the scene, so these aren't undo steps ----

	// "Camera at [x, y, z], looking along [x, y, z]" from the camera's current transform
	const auto viewCameraText = []() {
		auto vecText = [](const glm::vec3& v) {
			char text[64];
			snprintf(text, sizeof(text), "[%.2f, %.2f, %.2f]", v.x, v.y, v.z);
			return std::string(text);
		};
		auto& transform = g_editorCamera.getComponent<Transformation>();
		const glm::vec3 front = transform.getWorldRotation() * glm::vec3(0.f, 0.f, -1.f);
		return "Camera at " + vecText(transform.getWorldPosition()) + ", looking along " + vecText(front) +
			". Call screenshot_viewport to see it.";
	};

	server.addTool({
		"set_view_camera",
		"Moves the editor camera (what screenshot_viewport shows) to 'position', looking at 'target'; both are world [x, y, z].",
		objectSchema({
			{"position", { {"type", "array"}, {"items", { {"type", "number"} }} }},
			{"target", { {"type", "array"}, {"items", { {"type", "number"} }} }},
		}, { "position", "target" }),
		[viewCameraText](const json& args) {
			requireEditMode();
			const glm::vec3 position = vec3FromArgs(args, "position");
			const glm::vec3 target = vec3FromArgs(args, "target");
			if (glm::length(target - position) < 1e-4f)
				throw McpToolError("'position' and 'target' must be different points");

			auto* editorCamera = dynamic_cast<EditorCamera*>(g_editorCamera.getComponent<NativeScriptComponent>().script.get());
			if (!editorCamera)
				throw McpToolError("The editor camera isn't set up yet");

			editorCamera->lookAt(position, target);
			return mcpText(viewCameraText());
		}
	});

	server.addTool({
		"focus_entity",
		"Moves the editor camera so the entity and its children fill the view, like double-clicking it in the "
		"Hierarchy. Optional 'direction' [x, y, z] is the way the camera looks (default [-1, -0.7, -1]: from above, at an angle).",
		objectSchema({
			{"id", { {"type", "integer"} }},
			{"direction", { {"type", "array"}, {"items", { {"type", "number"} }} }},
		}, { "id" }),
		[viewCameraText](const json& args) {
			requireEditMode();
			Entity e = entityFromArgs(args);

			glm::vec3 direction{ -1.f, -0.7f, -1.f };
			if (args.contains("direction"))
				direction = vec3FromArgs(args, "direction");
			if (glm::length(direction) < 1e-4f)
				throw McpToolError("'direction' can't be [0, 0, 0]");

			focusOnEntity(e, g_editorCamera, direction);
			return mcpText("Framed " + describe(e) + ". " + viewCameraText());
		}
	});

	// ---- Mutations: each call is one undo step labelled "AI: ..." ----

	server.addTool({
		"create_entity",
		"Creates an entity with a name and a Transformation. Optional 'parent' (entity id) and 'components' in "
		"get_entity's form, e.g. {\"PointLight\": {...}}. Fields of Transformation can be partial; any other component "
		"needs all its fields (copy them from get_component_schema).",
		objectSchema({
			{"name", { {"type", "string"} }},
			{"parent", { {"type", "integer"}, {"description", "Entity id to create it under"} }},
			{"components", { {"type", "object"}, {"description", "Component name -> fields"} }},
		}, { "name" }),
		[](const json& args) {
			requireEditMode();
			const std::string name = args.value("name", "");
			if (name.empty())
				throw McpToolError("'name' is required");

			Entity parent = Entity::EmptyEntity;
			if (args.contains("parent") && !args["parent"].is_null())
				parent = entityFromArgs(args, "parent");

			// Held in a local: items() only references the object, it must outlive the loop
			const json components = args.value("components", json::object());

			auto scene = Engine::get()->getContext()->getActiveScene();
			Entity e = scene->createEntity(name);
			try
			{
				// All data first, then set it up, like any entity that's new to the scene
				for (auto& [component, fields] : components.items())
					Archiver::deserializeComponent(e, component, componentWithFields(e, component, fields));
				Archiver::initializeEntities({ e }, scene);
				checkHierarchyLinks(e);
			}
			catch (...)
			{
				// Don't leave a half-built entity behind
				e.remove();
				updateScene();
				throw;
			}

			std::vector<ComponentSnapshot> parentBefore;
			if (parent.valid())
			{
				parentBefore.push_back(takeSnapshot(parent, componentTypeName<Transformation>()));
				e.getComponent<Transformation>().setParent(parent);
			}

			CommandStack::Instance().record(std::make_unique<CreateEntitiesCmd>(e, "AI: create " + name, std::move(parentBefore)));
			updateScene();
			return mcpText("Created " + describe(e));
		}
	});

	// The preset names and descriptions come from the Hierarchy's Create menu table
	json presetNames = json::array();
	std::string presetList;
	for (const auto& preset : entityPresets())
	{
		presetNames.push_back(preset.name());
		presetList += "\n- " + preset.name() + " (" + preset.path + "): " + preset.description;
	}

	server.addTool({
		"create_preset",
		"Creates an entity the way the Hierarchy's Create menu does, with everything that item sets up. Optional "
		"'name' renames it, 'parent' (entity id) creates it under that entity, 'position' [x, y, z] places it "
		"relative to the parent (or the world without one). "
		"The reply lists its components and children. Presets:" + presetList,
		objectSchema({
			{"preset", { {"type", "string"}, {"enum", presetNames} }},
			{"name", { {"type", "string"} }},
			{"position", { {"type", "array"}, {"items", { {"type", "number"} }} }},
			{"parent", { {"type", "integer"}, {"description", "Entity id to create it under"} }},
		}, { "preset" }),
		[presetNames](const json& args) {
			requireEditMode();
			const std::string presetName = args.value("preset", "");
			const EntityPreset* preset = findPreset(presetName);
			if (!preset)
			{
				std::vector<std::string> names;
				for (const auto& name : presetNames)
					names.push_back(name.get<std::string>());
				throw McpToolError("Unknown preset '" + presetName + "'. Presets: " + joinNames(names));
			}

			// Parse everything first so a bad value doesn't leave an entity behind
			std::optional<glm::vec3> position;
			if (args.contains("position"))
				position = vec3FromArgs(args, "position");
			Entity parent = Entity::EmptyEntity;
			if (args.contains("parent") && !args["parent"].is_null())
				parent = entityFromArgs(args, "parent");
			const std::string name = args.value("name", "");

			Entity e = preset->create();
			if (!e.valid())
				throw McpToolError("Preset '" + presetName + "' created nothing; check get_console for the reason");

			if (!name.empty())
				e.getComponent<ObjectComponent>().name = name;

			std::vector<ComponentSnapshot> parentBefore;
			if (parent.valid())
			{
				parentBefore.push_back(takeSnapshot(parent, componentTypeName<Transformation>()));
				e.getComponent<Transformation>().setParent(parent);
			}

			// After parenting: setParent keeps the world position, and position is relative to the parent
			if (position)
				e.getComponent<Transformation>().setLocalPosition(*position);

			CommandStack::Instance().record(std::make_unique<CreateEntitiesCmd>(e, "AI: create " + preset->name(), std::move(parentBefore)));
			updateScene();

			// Held in a local: items() only references the object, it must outlive the loop
			const json components = componentsOf(e);
			std::vector<std::string> componentNames;
			for (auto& item : components.items())
				componentNames.push_back(item.key());

			std::vector<std::string> children;
			for (auto& child : childrenOf(e))
				children.push_back(describe(child));

			return mcpText("Created " + describe(e) + " from preset " + preset->name() + ". Components: " + joinNames(componentNames) +
				(children.empty() ? "" : ". Children: " + joinNames(children)));
		}
	});

	server.addTool({
		"delete_entity",
		"Deletes an entity and all its children.",
		objectSchema({
			{"id", { {"type", "integer"} }},
		}, { "id" }),
		[](const json& args) {
			requireEditMode();
			Entity e = entityFromArgs(args);
			const std::string description = describe(e);
			const size_t descendants = countDescendants(e);

			runOrThrow(std::make_unique<DeleteEntitiesCmd>(e, "AI: delete " + entityName(e)));
			return mcpText("Deleted " + description + (descendants ? " and " + std::to_string(descendants) + " descendants" : ""));
		}
	});

	server.addTool({
		"set_components",
		"Changes components of one entity. 'components' maps a component name to the fields to change, in get_entity's "
		"form; fields you leave out keep their current values. A component the entity lacks is added (give all its "
		"fields, see get_component_schema). null removes the component, except Transformation and ObjectComponent, "
		"which every entity needs (delete_entity removes the entity).",
		objectSchema({
			{"id", { {"type", "integer"} }},
			{"components", { {"type", "object"}, {"description", "Component name -> fields to change, or null to remove"} }},
		}, { "id", "components" }),
		[](const json& args) {
			requireEditMode();
			Entity e = entityFromArgs(args);
			if (!args["components"].is_object() || args["components"].empty())
				throw McpToolError("'components' must map component names to fields, e.g. {\"Transformation\": {...}}");

			std::vector<ComponentSnapshot> before, after;
			std::vector<std::string> names;
			for (auto& [component, fields] : args["components"].items())
			{
				// The editor and engine assume every entity has these (the Hierarchy, transform updates)
				if (fields.is_null() && (component == componentTypeName<Transformation>() || component == componentTypeName<ObjectComponent>()))
					throw McpToolError("Can't remove " + component + ": every entity needs it. Use delete_entity to remove the entity.");

				before.push_back(takeSnapshot(e, component));

				ComponentSnapshot target{ e.handlerID(), component };
				if (!fields.is_null())
				{
					target.present = true;
					target.data = componentWithFields(e, component, fields);
				}
				after.push_back(std::move(target));
				names.push_back(component);
			}

			// A Transformation written by hand can break the parent/child links, on this entity or on the ones
			// linked to it before (a parent still listing a child that left); refuse it instead of keeping it
			std::vector<entity_id> linked = { e.handlerID() };
			if (parentOf(e).valid())
				linked.push_back(parentOf(e).handlerID());
			for (auto& child : childrenOf(e))
				linked.push_back(child.handlerID());

			runOrThrow(std::make_unique<EditComponentsCmd>(std::move(before), std::move(after), "AI: set " + joinNames(names) + " on " + entityName(e),
				[linked] {
					for (entity_id id : linked)
					{
						if (Entity linkedEntity = entityFromId(id); linkedEntity.valid())
							checkHierarchyLinks(linkedEntity);
					}
				}));
			return mcpText("Updated " + joinNames(names) + " on " + describe(e));
		}
	});

	server.addTool({
		"set_transform",
		"Moves, rotates or scales an entity relative to its parent. position and scale are [x, y, z]; rotation is "
		"[x, y, z] Euler angles in degrees. Values you leave out stay as they are.",
		objectSchema({
			{"id", { {"type", "integer"} }},
			{"position", { {"type", "array"}, {"items", { {"type", "number"} }} }},
			{"rotation", { {"type", "array"}, {"items", { {"type", "number"} }} }},
			{"scale", { {"type", "array"}, {"items", { {"type", "number"} }} }},
		}, { "id" }),
		[](const json& args) {
			requireEditMode();
			Entity e = entityFromArgs(args);
			if (!args.contains("position") && !args.contains("rotation") && !args.contains("scale"))
				throw McpToolError("Give at least one of 'position', 'rotation', 'scale'");

			// Parse everything first so a bad value doesn't leave the transform half changed
			std::optional<glm::vec3> position, rotation, scale;
			if (args.contains("position")) position = vec3FromArgs(args, "position");
			if (args.contains("rotation")) rotation = vec3FromArgs(args, "rotation");
			if (args.contains("scale")) scale = vec3FromArgs(args, "scale");

			const std::string component = componentTypeName<Transformation>();
			ComponentSnapshot before = takeSnapshot(e, component);

			auto& transform = e.getComponent<Transformation>();
			if (position) transform.setLocalPosition(*position);
			if (rotation) transform.setLocalRotation(glm::quat(glm::radians(*rotation)));
			if (scale) transform.setLocalScale(*scale);

			CommandStack::Instance().record(std::make_unique<EditComponentsCmd>(
				std::vector<ComponentSnapshot>{ before }, std::vector<ComponentSnapshot>{ takeSnapshot(e, component) },
				"AI: transform " + entityName(e)));
			return mcpText("Transformed " + describe(e));
		}
	});

	server.addTool({
		"rename_entity",
		"Renames an entity.",
		objectSchema({
			{"id", { {"type", "integer"} }},
			{"name", { {"type", "string"} }},
		}, { "id", "name" }),
		[](const json& args) {
			requireEditMode();
			Entity e = entityFromArgs(args);
			const std::string name = args.value("name", "");
			if (name.empty())
				throw McpToolError("'name' must not be empty");
			if (!e.HasComponent<ObjectComponent>())
				throw McpToolError("Entity " + std::to_string(e.handlerID()) + " has no ObjectComponent to hold a name");

			const std::string component = componentTypeName<ObjectComponent>();
			const std::string oldName = entityName(e);
			ComponentSnapshot before = takeSnapshot(e, component);
			e.getComponent<ObjectComponent>().name = name;

			CommandStack::Instance().record(std::make_unique<EditComponentsCmd>(
				std::vector<ComponentSnapshot>{ before }, std::vector<ComponentSnapshot>{ takeSnapshot(e, component) },
				"AI: rename " + oldName + " to " + name));
			updateScene();
			return mcpText("Renamed id " + std::to_string(e.handlerID()) + " from \"" + oldName + "\" to \"" + name + "\"");
		}
	});

	server.addTool({
		"set_parent",
		"Moves an entity under another one, keeping its world position. 'parent' null moves it to the scene root.",
		objectSchema({
			{"id", { {"type", "integer"} }},
			{"parent", { {"type", {"integer", "null"}} }},
		}, { "id", "parent" }),
		[](const json& args) {
			requireEditMode();
			Entity child = entityFromArgs(args);

			Entity parent = Entity::EmptyEntity;
			if (!args["parent"].is_null())
				parent = entityFromArgs(args, "parent");

			for (Entity p = parent; p.valid(); p = parentOf(p))
			{
				if (p == child)
					throw McpToolError("Can't put an entity under itself or one of its descendants");
			}

			const std::string label = parent.valid()
				? "AI: move " + entityName(child) + " under " + entityName(parent)
				: "AI: move " + entityName(child) + " to root";

			if (!reparentWithUndo(child, parent, label))
				return mcpText("Nothing changed: " + describe(child) + " already has that parent");

			return mcpText(describe(child) + (parent.valid() ? " is now a child of " + describe(parent) : " is now at the scene root"));
		}
	});

	server.addTool({
		"instantiate_prefab",
		"Creates the entities of a prefab asset (by uuid or name from list_assets) at an optional [x, y, z] position.",
		objectSchema({
			{"uuid", { {"type", "string"} }},
			{"name", { {"type", "string"} }},
			{"position", { {"type", "array"}, {"items", { {"type", "number"} }} }},
		}),
		[](const json& args) {
			requireEditMode();
			const AssetRecord& asset = findAssetRecord(args);
			if (asset.aType != AssetType::PREFAB)
				throw McpToolError("\"" + asset.name + "\" is a " + getAssetTypeAsStr(asset.aType) + " asset, not a prefab");

			const glm::vec3 position = args.contains("position") ? vec3FromArgs(args, "position") : glm::vec3(0.0f);

			PrefabAssetRef prefab = Engine::get()->getSubSystem<Assets>()->getAsset(asset.uuid).as<PrefabAsset>();
			if (prefab.isEmpty())
				throw McpToolError("Couldn't load prefab \"" + asset.name + "\"");

			Entity root = prefab.resource()->Instansiate(position);
			CommandStack::Instance().record(std::make_unique<CreateEntitiesCmd>(root, "AI: instantiate " + asset.name));
			updateScene();
			return mcpText("Instantiated \"" + asset.name + "\" as " + describe(root));
		}
	});

	server.addTool({
		"undo",
		"Undoes the last editor action, yours or the user's (the same history as Ctrl+Z).",
		objectSchema(),
		[](const json&) {
			requireEditMode();
			auto& stack = CommandStack::Instance();
			if (!stack.canUndo())
				return mcpText("Nothing to undo");

			const std::string label = stack.undoLabel();
			if (!stack.undo())
				throw McpToolError("Undoing \"" + label + "\" failed; see get_console");
			return mcpText("Undid \"" + label + "\"");
		}
	});

	server.addTool({
		"redo",
		"Redoes the last undone action (the same history as Ctrl+Y).",
		objectSchema(),
		[](const json&) {
			requireEditMode();
			auto& stack = CommandStack::Instance();
			if (!stack.canRedo())
				return mcpText("Nothing to redo");

			const std::string label = stack.redoLabel();
			if (!stack.redo())
				throw McpToolError("Redoing \"" + label + "\" failed; see get_console");
			return mcpText("Redid \"" + label + "\"");
		}
	});

	server.addTool({
		"save_scene",
		"Saves the project, including the open scene, to disk.",
		objectSchema(),
		[](const json&) {
			requireEditMode();
			Engine::get()->saveProject();
			return mcpText("Saved the project");
		}
	});

	// ---- Scenes: switching clears undo history, like opening a scene from the Asset View ----

	server.addTool({
		"new_scene",
		"Creates a scene like the New Scene dialog (a directional light and a main camera) and opens it.",
		objectSchema({
			{"name", { {"type", "string"} }},
		}, { "name" }),
		[](const json& args) {
			requireEditMode();
			const std::string name = args.value("name", "");
			const std::string nameError = UniqueNameManager::fileNameError(name);
			if (!nameError.empty())
				throw McpToolError("The scene name " + nameError + " (it becomes the .scene file name)");

			for (const AssetRecord* record : Engine::get()->getSubSystem<Assets>()->getAllRecords())
			{
				if (record && record->aType == AssetType::SCENE && record->name == name)
					throw McpToolError("A scene named '" + name + "' already exists; use open_scene to open it");
			}

			const std::string previous = Engine::get()->getContext()->getActiveSceneAsset().info().name;
			openSceneInEditor(StartupSceneTemplate::createStartupScene(name));

			std::string reply = "Created and opened scene '" + name + "'.";
			if (!previous.empty())
				reply += " '" + previous + "' may have unsaved changes.";
			return mcpText(reply);
		}
	});

	server.addTool({
		"open_scene",
		"Opens a scene by 'name' or 'uuid' (see list_assets with type 'scene'), like Open in the Asset View.",
		objectSchema({
			{"name", { {"type", "string"} }},
			{"uuid", { {"type", "string"} }},
		}),
		[](const json& args) {
			requireEditMode();
			const AssetRecord& record = findAssetRecord(args);
			if (record.aType != AssetType::SCENE)
				throw McpToolError("'" + record.name + "' has type '" + getAssetTypeAsStr(record.aType) + "', not 'scene'. Use list_assets with type 'scene'.");

			SceneAssetRef scene = Engine::get()->getSubSystem<Assets>()->getAsset(record.uuid).as<SceneAsset>();
			if (scene.isEmpty())
				throw McpToolError("Scene '" + record.name + "' couldn't be loaded; see get_console");

			const std::string previous = Engine::get()->getContext()->getActiveSceneAsset().info().name;
			openSceneInEditor(scene);

			std::string reply = "Opened scene '" + record.name + "'.";
			if (!previous.empty() && previous != record.name)
				reply += " '" + previous + "' may have unsaved changes.";
			return mcpText(reply);
		}
	});
}
