#include "EntryPoint.h"
#include "sge.h"

#include "commdlg.h"

#include <filesystem>

#include "imgui_internal.h"

#include "tinyfiledialogs.h"

#include "NativeScriptsLoader.h"
#include "UIEventLayer.h"

#include "EditorCamera.h"
#include "EditorState.h"

#include <imgui_stdlib.h>
#include "core/Logger.h"
#include <vector>
#include <mutex>

#include "Common.h"
#include "AssetViewWindow.h"

#include "EntityState.h"
#include "InspectorWindow.h"
#include "Dialogs.h"

static const std::string SGE_EDITOR_APP_ROOT = "../../EditorApp/Resources";
std::shared_ptr<EventLayer> uiLayer = std::make_shared<UIEventLayer>();

namespace fs = std::filesystem;

struct Message
{
	spdlog::level::level_enum level;
	std::string msg;
};

inline ImVec4 LogLevelToColor(spdlog::level::level_enum level)
{
	switch (level)
	{
	case spdlog::level::trace:
		return ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // light gray
	case spdlog::level::debug:
		return ImVec4(0.3f, 0.8f, 0.3f, 1.0f); // green
	case spdlog::level::info:
		return ImVec4(0.2f, 0.6f, 1.0f, 1.0f); // blue
	case spdlog::level::warn:
		return ImVec4(1.0f, 0.8f, 0.2f, 1.0f); // yellow/orange
	case spdlog::level::err:
		return ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // red
	case spdlog::level::critical:
		return ImVec4(0.9f, 0.1f, 0.1f, 1.0f); // dark red
	case spdlog::level::off:
		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // white (disabled)
	default:
		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // fallback white
	}
}

// Console log storage
static std::vector<Message> g_consoleLog;
static std::mutex g_consoleMutex;
static bool g_scrollConsole = false;

static void appendConsoleLog(spdlog::level::level_enum level, const std::string& msg)
{
	std::lock_guard<std::mutex> lock(g_consoleMutex);
	g_consoleLog.push_back({ level, msg });
	g_scrollConsole = true;
}

class ConsoleLoggerRegister
{
public:
	ConsoleLoggerRegister()
	{
		Logger::setCallback(appendConsoleLog);
	}
};

static ConsoleLoggerRegister clr;
Entity g_primaryCamera;
Entity g_editorCamera;

uint32_t g_previewWindowID = 0;

static void stopSimulation()
{
	EditorState::Instance().startButtonPressed = false; // Toggle the state
	Engine::get()->getContext()->getActiveScene()->stopSimulation();

	//Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(g_editorCamera);

	uiLayer->setEnabled(true);
	static_cast<EditorCamera*>(g_editorCamera.getComponent<NativeScriptComponent>().script.get())->unlock(); //TODO this should be in camera event
}

static void startsimulation()
{
	EditorState::Instance().startButtonPressed = true; // Toggle the state
	Engine::get()->getContext()->getActiveScene()->startSimulation();

	//Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(g_primaryCamera);

	uiLayer->setEnabled(false);
	state.selectEntity(Entity::EmptyEntity);
	static_cast<EditorCamera*>(g_editorCamera.getComponent<NativeScriptComponent>().script.get())->lock(); //TODO this should be in camera event

}

void focusOnEntity(Entity e)
{
	// get camera forward
	auto& camera = g_editorCamera.getComponent<CameraComponent>();
	auto front = camera.front;

	// get entity location
	auto& targetTransform = e.getComponent<Transformation>();
		
	glm::vec3 targetLocation = targetTransform.getWorldPosition() - front * 5.f;

	auto targetMesh = e.tryGetComponent<MeshComponent>();
	if (targetMesh)
	{
		auto& targetAABB = targetMesh->mesh.get()->getPrimaryMesh()->getAABB();

		// set destination to location + forward
		targetLocation = targetTransform.getWorldPosition() - front * 5.f + targetAABB.extents * .5f;

// create fake frustum
Frustum fakeFrustum(targetLocation + front * 10.f, front, camera.up, camera.right, camera.aspect, camera.fovy, camera.znear, camera.zfar);

// we start at the target object location and step back until the object AABB is inside the frustum.
while (!targetAABB.isOnFrustum(fakeFrustum))
{
	targetLocation -= front;
	fakeFrustum = Frustum(targetLocation + front * 10.f, front, camera.up, camera.right, camera.aspect, camera.fovy, camera.znear, camera.zfar);
}
	}

	auto& transform = g_editorCamera.getComponent<Transformation>();
	transform.setLocalPosition(targetLocation);
}

void RenderSimulationControlView()
{
	ImGui::Begin("Simulation Controls", nullptr, windowFlags | ImGuiWindowFlags_NoResize);

	float windowWidth = ImGui::GetContentRegionAvail().x;
	ImGui::SetCursorPosX((windowWidth - 100) * 0.5f);

	// Draw the button based on the current state
	if (EditorState::Instance().startButtonPressed)
	{
		if (ImGui::Button("STOP", ImVec2(70, 0)))
		{
			// Handle stop button click
			stopSimulation();
		}
	}
	else
	{
		if (ImGui::Button("START", ImVec2(70, 0)))
		{
			// Handle start button click
			startsimulation();
		}
	}

	ImGui::End(); // End the window
}                                           

std::string OpenFile(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	CHAR currentDir[256] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)Engine::get()->getWindow()->GetNativeWindow();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn) == TRUE)
		return ofn.lpstrFile;

	return std::string();
}

std::string SaveFile(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	CHAR currentDir[256] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)Engine::get()->getWindow()->GetNativeWindow();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetSaveFileNameA(&ofn) == TRUE)
		return ofn.lpstrFile;

	return std::string();
}

void LightCreatorWindow()
{
	if (EditorState::Instance().showLightCreatorWindow)
	{
		ImGui::SetNextWindowSize({ 400, 300 }, ImGuiCond_Appearing);
		ImGui::Begin("Light Creator");

		static LightType lightType = LightType::DirectionalLight;
		static float ambientIntensity = 0.2f;
		static float diffuseIntensity = 0.5f;
		static glm::vec3 color(1.f, 1.f, 1.f);
		static glm::vec3 dir(0.f, 0.f, 0.f);
		static glm::vec3 pos(0.f, 0.f, 0.f);
		static Attenuation attenuation;

		// Light Type
		ImGui::RadioButton("Directional Light", (int*)&lightType, 0);
		ImGui::RadioButton("Point Light", (int*)&lightType, 1);

		ImGui::ColorEdit3("Color", (float*)&color);
		ImGui::SliderFloat("Ambient intensity", &ambientIntensity, 0.0f, 1.0f);
		ImGui::SliderFloat("Diffuse intensity", &diffuseIntensity, 0.0f, 1.0f);

		if (lightType == LightType::DirectionalLight)
		{
			ImGui::InputFloat3("Direction", (float*)&dir);
		}
		if (lightType == LightType::PointLight)
		{
			ImGui::InputFloat3("Position", (float*)&pos);
			ImGui::LabelText("", "Attenuation");
			ImGui::SliderFloat("constant", (float*)&attenuation.constant, 0.f, 1.f);
			ImGui::SliderFloat("linear", (float*)&attenuation.linear, 0.f, 1.f);
			ImGui::SliderFloat("quadratic", (float*)&attenuation.quadratic, 0.f, 1.f);
		}

		if (ImGui::Button("Ok"))
		{
			if (lightType == LightType::DirectionalLight)
			{
				auto e = Engine::get()->getContext()->getActiveScene()->createEntity();
				e.addComponent<DirectionalLight>(color, dir, ambientIntensity, diffuseIntensity);

			}
			else if (lightType == LightType::PointLight)
			{
				auto e = Engine::get()->getContext()->getActiveScene()->createEntity();
				e.addComponent<PointLight>(color, ambientIntensity, diffuseIntensity, attenuation);
			}

			EditorState::Instance().showLightCreatorWindow = false;

			updateScene();

			//logInfo("Added light successfully.");
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			EditorState::Instance().showLightCreatorWindow = false;
		}

		ImGui::End();
	}

}

static void addAssetLoadWidget(const std::string& name, ImGuiTextBuffer& textBuffer, const char** assetSupportedFormats, int filterCount)
{
	ImGui::LabelText("", name.c_str());
	if (ImGui::Button(std::string("Browse##" + name).c_str()))
	{
		const char* filepath = tinyfd_openFileDialog(
			"Select an asset to load",
			"",
			filterCount,
			assetSupportedFormats,
			"",
			0);

		if (filepath)
		{
			textBuffer.clear();
			textBuffer.append(filepath);
		}
	}
	ImGui::SameLine();
	ImGui::TextUnformatted(textBuffer.begin(), textBuffer.end());
}

void ShowTextureImportWindow()
{
	static ImGuiTextBuffer texturePathBuffer;

	const char* filepath = tinyfd_openFileDialog(
		"Select an asset to load",
		"",
		4,
		Constants::g_textureSupportedFormats,
		"",
		0);

	if (filepath)
	{
		texturePathBuffer.clear();
		texturePathBuffer.append(filepath);
	}

	std::filesystem::path path(texturePathBuffer.c_str());

	if (!std::filesystem::exists(path))
	{
		logError("Texture Path not found: " + path.string());
		return;
	}

	Texture::import(texturePathBuffer.c_str());

	texturePathBuffer.clear();
}

void ShowAnimationImportWindow()
{
	static ImGuiTextBuffer animationPathBuffer;

	const char* filepath = tinyfd_openFileDialog(
		"Select an asset to load",
		"",
		1,
		Constants::g_animationSupportedFormats,
		"",
		0);

	if (filepath)
	{
		animationPathBuffer.clear();
		animationPathBuffer.append(filepath);
	}

	std::filesystem::path path(animationPathBuffer.c_str());

	if (!std::filesystem::exists(path))
	{
		logError("Animation Path not found: " + path.string());
		return;
	}

	Animation::import(animationPathBuffer.c_str());
	//Engine::get()->getSubSystem<AnimationLoader>()->import(animationPathBuffer.c_str());

	animationPathBuffer.clear();
}

void ShowModelCreatorWindow()
{
	static ImGuiTextBuffer modelPathBuffer;

	const char* filepath = tinyfd_openFileDialog(
		"Select an asset to load",
		"",
		5,
		Constants::g_supportedFormats,
		"",
		0);

	if (filepath)
	{
		modelPathBuffer.clear();
		modelPathBuffer.append(filepath);
	}

	std::filesystem::path path(modelPathBuffer.c_str());

	if (!std::filesystem::exists(path))
	{
		logError("Model Path not found: " + path.string());
		return;
	}

	auto modelName = path.filename().replace_extension().string();

	auto entity = Engine::get()->getContext()->getActiveScene()->createEntity(modelName);
	entity.addComponent<RenderableComponent>();

	auto mesh = MeshCollection::import(modelPathBuffer.c_str());
	entity.addComponent<MeshComponent>().mesh = mesh;

	auto& materials = MeshCollection::getLastLoadedMaterials();

	auto& materialComponent = entity.addComponent<MaterialComponent>();
	for(auto& [idx, m] : materials)
	{
		materialComponent.setMaterial(idx, m);
	}

	AssetInfo aInfo;
	aInfo.name = modelName;
	Prefab::create(entity, aInfo);

	entity.remove();

	modelPathBuffer.clear();

	updateScene();
}

void displayentityName(const Entity& e)
{
	auto& obj = e.getComponent<ObjectComponent>();

	ImGui::Selectable(obj.name.c_str(), (state.getSelectedEntity() == e));

	if (ImGui::IsItemClicked())
	{
		state.selectEntity(e);
	}
}

void displayEntityHelper(Entity& e)
{
	auto& transform = e.getComponent<Transformation>();
	auto& obj = e.getComponent<ObjectComponent>();

	

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		state.selectEntity(e);
	}

	if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
	{
		state.selectEntity(e);
		ImGui::OpenPopup("SceneObjectContextPopup");
	}

	

	if (state.getSelectedEntity() == e)
	{
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		{
			if (ImGui::IsMouseDoubleClicked(0))
			{
				focusOnEntity(e);
			}
		}

		//if (selectedEntityRename)
		//{
		//	// Editable text field
		//	char buffer[256];
		//	strncpy(buffer, obj.name.c_str(), sizeof(buffer));
		//	buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination

		//	if (ImGui::InputText("##edit", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		//	{
		//		bool isValid = true;
		//		for (int j = 0; j < sceneObjects.size(); j++)
		//		{
		//			if (sceneObjects[j].e == e) continue;

		//			if (sceneObjects[j].name == buffer)
		//			{
		//				logError("Cannot rename to already existing name.");
		//				isValid = false;
		//			}
		//		}

		//		if (isValid)
		//		{
		//			obj.name = buffer;
		//			updateScene(); // Assuming this updates any necessary scene state
		//			selectedEntityRename = false;
		//		}
		//	}
		//}

		if (ImGui::BeginPopup("SceneObjectContextPopup"))
		{
			if (ImGui::MenuItem("Rename"))
			{
				// Focus the input text when renaming
				EditorState::Instance().selectedEntityRename = true;
			}

			if (ImGui::MenuItem("Create Prefab"))
			{
				std::string assetName = state.getSelectedEntity().getComponent<ObjectComponent>().name;

				AssetInfo aInfo;
				aInfo.name = assetName;
				auto prefab = Prefab::create(state.getSelectedEntity(), aInfo);
			}

			if (e.HasComponent<CameraComponent>())
			{
				if (ImGui::MenuItem("Set as Primary Camera"))
				{
					auto scene = Engine::get()->getContext()->getActiveScene();
					scene->setGameCamera(e);
				}
			}

			if (ImGui::MenuItem("Remove from parent")) // todo fix this nonsense
			{
				e.getComponent<Transformation>().removeParent();
			}

			if (ImGui::MenuItem("Delete"))
			{
				e.remove();
				updateScene();
				state.selectEntity(Entity::EmptyEntity);
			}

			ImGui::EndPopup();

		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("DND_ITEM", &e, sizeof(Entity));
			ImGui::Text("Dragging %s", obj.name.c_str());
			ImGui::EndDragDropSource();
		}
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM"))
		{
			Entity sourceEntity = *(const Entity*)payload->Data;
			if (sourceEntity != e)
			{
				sourceEntity.getComponent<Transformation>().setParent(e);
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(150.0f);
	if (EditorState::Instance().selectedEntityRename && state.getSelectedEntity() == e)
	{
		ImGui::SetKeyboardFocusHere();
		std::string& renameText = state.getCurrentEntityState().renameBuffer;

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0)); // Fully transparent background
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		ImGui::InputText(("##edit_" + obj.name).c_str(), &renameText);

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3); // Pop all three

		if (ImGui::IsItemDeactivated())
		{
			obj.name = renameText;
			updateScene(); // Assuming this updates any necessary scene state
			EditorState::Instance().selectedEntityRename = false;
		}
		//ImGui::InputText("##edit", &yourNameString);
	}
	else
	{
		ImGui::Text(obj.name.c_str());
	}
}

void displayEntity(Entity& e)
{
	auto& transform = e.getComponent<Transformation>();
	auto& obj = e.getComponent<ObjectComponent>();
	bool hasChildren = transform.getChildren().size() > 0;

	static int nonLeafTreeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
	static int leafTreeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;

	ImGui::SetNextItemWidth(300.0f);

	
	if (hasChildren)
	{
		int flags = nonLeafTreeFlags;
		flags = (state.getSelectedEntity() == e) ? flags | ImGuiTreeNodeFlags_Selected : flags;
		bool isOpen = ImGui::TreeNodeEx(("##" + obj.name).c_str(), flags);

		displayEntityHelper(e);

		if (isOpen)
		{
			//ImGui::TreePush(obj.name.c_str());
			auto childrens = transform.getChildren();
			auto childIter = childrens.begin();
			while (childIter != childrens.end())
			{
				displayEntity(childIter->second);
				childIter++;
			}
			//ImGui::TreePop();
			ImGui::TreePop();
		}
	}
	else
	{
		int flags = leafTreeFlags;
		flags = (state.getSelectedEntity() == e) ? flags | ImGuiTreeNodeFlags_Selected : flags;
		ImGui::TreeNodeEx(("##" + obj.name).c_str(), flags);
		displayEntityHelper(e);
	}

	
}

void displaySceneObjects()
{
	// todo fix this shitty hack
	auto sceneName = "Scene " + std::to_string(Engine::get()->getContext()->getActiveSceneID());
	ImGui::Text(sceneName.c_str());

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM"))
		{
			Entity sourceEntity = *(const Entity*)payload->Data;
			sourceEntity.getComponent<Transformation>().removeParent();
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Separator();

	ImGui::BeginChild("##items", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_NoScrollbar);

	for (int i = 0; i < sceneObjects.size(); ++i)
	{
		auto& sceneObject = sceneObjects[i];
		auto& transform = sceneObject.e.getComponent<Transformation>();

		// if has parent it will be rendered in the recursive call (can be optimized if needed)
		if (transform.getParent().valid()) 
			continue;

		ImGui::PushID(i); // Push a unique ID to avoid ImGui ID conflicts
		
		displayEntity(sceneObject.e);

		ImGui::PopID();
	}

	ImGui::EndChild(); // End background drop zone
}

void RenderSceneHierarchyWindow()
{
	ImGui::Begin("Scene Hierarchy", nullptr, windowFlags);

	//displayWindowHeader("Scene Hierarchy");

	float windowWidth = ImGui::GetContentRegionAvail().x;
	if (ImGui::Button("+", ImVec2(windowWidth, 0)))
	{
		ImGui::OpenPopup("AddObjectToScenePopup");
	}

	if (ImGui::BeginPopup("AddObjectToScenePopup")) 
	{
		if (ImGui::MenuItem("Entity"))
		{
			Engine::get()->getContext()->getActiveScene()->createEntity();
			updateScene();
			state.selectEntity(sceneObjects[0].e);
		}
		if (ImGui::BeginMenu("Primitive")) 
		{ // Begin the submenu
			if (ImGui::MenuItem("Cube")) 
			{
				ShapeFactory::createBoxEntity(&Engine::get()->getContext()->getActiveScene()->getRegistry());
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}
			if (ImGui::MenuItem("Sphere")) 
			{
				ShapeFactory::createSphere(&Engine::get()->getContext()->getActiveScene()->getRegistry());
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}
			if (ImGui::MenuItem("Quad")) 
			{
				ShapeFactory::createQuad(&Engine::get()->getContext()->getActiveScene()->getRegistry());
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			ImGui::EndMenu(); // End the submenu
		}

		if (ImGui::BeginMenu("Light"))
		{ // Begin the submenu
			if (ImGui::MenuItem("Directional Light"))
			{
				static int createdDLightCount = 0;
				auto e = Engine::get()->getContext()->getActiveScene()->createEntity("Directional_Light_" + std::to_string(createdDLightCount++));
				e.addComponent<DirectionalLight>(glm::vec3{ 0,0,0 }, glm::vec3{0,-1,0}, 1.f, 1.f);
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}
			if (ImGui::MenuItem("Point Light"))
			{
				static int createdPLightCount = 0;
				auto e = Engine::get()->getContext()->getActiveScene()->createEntity("Point_Light_" + std::to_string(createdPLightCount++));
				e.addComponent<PointLight>(glm::vec3{ 0,0,0 }, 1.f, 1.f, Attenuation());
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			ImGui::EndMenu(); // End the submenu
		}

		if (ImGui::BeginMenu("Environment"))
		{ // Begin the submenu
			if (ImGui::MenuItem("Skybox"))
			{
				Entity e = Skybox::createSkybox(SGE_ROOT_DIR + "ResourceWrappers/Engine/Textures/sunflowers_puresky_4k.hdr", Skybox::TexType::EQUIRECTANGULAR);
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			if (ImGui::MenuItem("Terrain"))
			{
				Entity e = Terrain::createTerrain(100, 100, 1, Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::BLACK_1X1));
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			if (ImGui::MenuItem("Pool"))
			{
				Entity e = WaterSystem::createPool();
				updateScene();
				state.selectEntity(sceneObjects[0].e);
			}

			ImGui::EndMenu(); // End the submenu
		}

		// Add more submenus or menu items as needed
		ImGui::EndPopup();
	}

	ImVec2 listBoxSize = ImGui::GetContentRegionAvail();

	// Render list view
	if (ImGui::BeginListBox("##Objects", listBoxSize))
	{
		// Iterate through each scene object and render it as a selectable item in the list
		displaySceneObjects();
		ImGui::EndListBox();
	}

	ImGui::End();
}

void RenderViewWindow()
{
	ImGui::Begin("View", nullptr, windowFlags);

	ImVec2 windowPos = ImGui::GetWindowPos();

	// Get the current window size to support resizing
	ImVec2 renderViewWindowSize = ImGui::GetContentRegionAvail();

	// Display the texture
	ImVec2 imageSize(renderViewWindowSize.x, renderViewWindowSize.y);

	unsigned int activeViewID = 0;
	if (Engine::get()->getContext()->getActiveScene()->isSimulationActive())
	{
		activeViewID = Engine::get()->getContext()->getActiveScene()->getGameRenderViewTextureID();
	}
	else
	{
		activeViewID = Engine::get()->getContext()->getActiveScene()->getRenderViewTextureID("Editor View");
	}

	ImGui::Image(reinterpret_cast<ImTextureID>(activeViewID), imageSize, ImVec2(0, 1), ImVec2(1, 0));

	// Calculate the top left position of the rendered image within the window
	ImVec2 viewportOffset = ImGui::GetWindowContentRegionMin();
	ImVec2 viewportPos{ windowPos.x + viewportOffset.x, windowPos.y + viewportOffset.y };

	ImVec2 mousePos = ImGui::GetMousePos();

	// Check if the mouse is within the viewport bounds
	if (mousePos.x >= viewportPos.x && mousePos.x <= viewportPos.x + renderViewWindowSize.x &&
		mousePos.y >= viewportPos.y && mousePos.y <= viewportPos.y + renderViewWindowSize.y)
	{
		EditorState::Instance().isMouseInSceneView = true;
	}
	else
	{
		EditorState::Instance().isMouseInSceneView = false;
	}

	if (!Engine::get()->getContext()->getActiveScene()->isSimulationActive())
	{
		// Define the size and position of the inner window
		float innerWindowWidth = renderViewWindowSize.x;
		float innerWindowHeight = 35.0f;
		ImVec2 toolbarPos(windowPos.x + 10, windowPos.y + 30);

		bool isPopupOpen = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);

		if (!isPopupOpen && !ImGuizmo::IsUsing() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 viewportOffset = ImGui::GetWindowContentRegionMin();
			ImVec2 viewportPos{ windowPos.x + viewportOffset.x, windowPos.y + viewportOffset.y };

			bool mouseInsideViewport = (mousePos.x >= viewportPos.x && mousePos.x <= viewportPos.x + renderViewWindowSize.x &&
				mousePos.y >= viewportPos.y && mousePos.y <= viewportPos.y + renderViewWindowSize.y);
			bool mouseInsideToolbar = (mousePos.x >= toolbarPos.x && mousePos.x <= toolbarPos.x + innerWindowWidth &&
				mousePos.y >= toolbarPos.y && mousePos.y <= toolbarPos.y + innerWindowHeight);

			if (mouseInsideViewport && !mouseInsideToolbar)
			{
				// We alter the mouse position from small window into full screen (the renderered object pick texture)
				int alteredX = (mousePos.x - viewportPos.x) / renderViewWindowSize.x * Engine::get()->getWindow()->getWidth();
				int alteredY = (mousePos.y - viewportPos.y) / renderViewWindowSize.y * Engine::get()->getWindow()->getHeight();
				int selectedID = Engine::get()->getSubSystem<ObjectPicker>()->pickObject(alteredX, alteredY, g_editorCamera);

				if (selectedID == -1)
				{
					state.selectEntity(Entity::EmptyEntity);

				}
				else
				{

					for (auto& sceneObj : sceneObjects)
					{
						if (sceneObj.e.handlerID() == selectedID)
						{
							state.selectEntity(sceneObj.e);
							break;
						}
					}
				}
			}
		}

		ImGui::SetNextWindowPos(toolbarPos); // Adjust position as needed
		ImGui::SetNextWindowSize(ImVec2(innerWindowWidth, innerWindowHeight)); // Adjust size as needed

		// Transformation mode enum and current mode variable
		enum TransformMode { TRANSLATE, ROTATE, SCALE, UNIVERSAL };
		static TransformMode currentMode = TRANSLATE;

		// Gizmo mode variable
		static ImGuizmo::MODE currentGizmoMode = ImGuizmo::LOCAL;

		// Snap options
		static bool useSnap = false;
		static float snapValues[3] = { 1.0f, 1.0f, 1.0f };

		

		if (ImGui::BeginChild("TransformWindow", ImVec2(innerWindowWidth, innerWindowHeight), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
		{
			// Radio buttons for transformation mode
			ImGui::RadioButton("Translate", (int*)&currentMode, TRANSLATE);
			ImGui::SameLine();
			ImGui::RadioButton("Rotate", (int*)&currentMode, ROTATE);
			ImGui::SameLine();
			ImGui::RadioButton("Scale", (int*)&currentMode, SCALE);
			ImGui::SameLine();
			ImGui::RadioButton("Universal", (int*)&currentMode, UNIVERSAL);

			// Button to toggle between local and world gizmo modes
			ImGui::SameLine();
			if (ImGui::Button(currentGizmoMode == ImGuizmo::LOCAL ? "Local" : "World"))
			{
				currentGizmoMode = (currentGizmoMode == ImGuizmo::LOCAL) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
			}

			// Checkbox for snap
			ImGui::SameLine();
			ImGui::Checkbox("Snap", &useSnap);

			// Input fields for snap values
			ImGui::SameLine();
			float snapInputWidth = 80.0f;
			if (currentMode == TRANSLATE)
			{
				ImGui::SetNextItemWidth(snapInputWidth * 3);
				ImGui::InputFloat3("Snap Translate", snapValues);
			}
			else if (currentMode == ROTATE)
			{
				ImGui::SetNextItemWidth(snapInputWidth);
				ImGui::InputFloat("Snap Angle", &snapValues[0]);
			}
			else if (currentMode == SCALE)
			{
				ImGui::SetNextItemWidth(snapInputWidth);
				ImGui::InputFloat("Snap Scale", &snapValues[0]);
			}

			ImGui::SameLine();

			static const char* renderModeOptions[] = { "Shaded", "Wireframe"};
			static int currentItem = 0; // Index of the selected item

			ImGui::SetCursorPosX(renderViewWindowSize.x - 170); // Adjust 200 to match the width of the dropdown
			ImGui::PushItemWidth(150.0f); // Set dropdown width to 150
			if (ImGui::BeginCombo("##RenderMode", renderModeOptions[currentItem])) // Label for the combo box
			{
				for (int i = 0; i < IM_ARRAYSIZE(renderModeOptions); i++)
				{
					bool isSelected = (currentItem == i);
					if (ImGui::Selectable(renderModeOptions[i], isSelected))
					{
						currentItem = i; // Update selected index
						Engine::get()->getContext()->setRenderMode((RenderMode)currentItem);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus(); // Set focus to the current item
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth(); // Restore default width
			ImGui::EndChild(); // End the inner window
		}

		if (state.getSelectedEntity() != Entity::EmptyEntity)
		{
			auto& transform = state.getSelectedEntity().getComponent<Transformation>();

			glm::mat4 glmMat = transform.getWorldTransformation();
			float* matrixPtr = glm::value_ptr(glmMat);

			auto& primaryCamera = g_editorCamera.getComponent<CameraComponent>();
			auto& primaryCameraTransform = g_editorCamera.getComponent<Transformation>();

			auto camView = glm::lookAt(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getWorldPosition() + primaryCamera.front, primaryCamera.up);
			const float* camViewPtr = glm::value_ptr(camView);

			auto projection = Engine::get()->getContext()->getActiveScene()->getProjection();
			const float* projectionPtr = glm::value_ptr(projection);

			ImGuizmo::SetDrawlist();
			ImVec2 winPos = ImGui::GetWindowPos();
			ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
			ImGuizmo::SetRect(winPos.x + contentMin.x, winPos.y + contentMin.y, renderViewWindowSize.x, renderViewWindowSize.y);

			// Set the operation mode based on the selected radio button
			ImGuizmo::OPERATION operationMode = ImGuizmo::TRANSLATE;
			switch (currentMode)
			{
			case TRANSLATE:
				operationMode = ImGuizmo::TRANSLATE;
				break;
			case ROTATE:
				operationMode = ImGuizmo::ROTATE;
				break;
			case SCALE:
				operationMode = ImGuizmo::SCALE;
				break;
			case UNIVERSAL:
				operationMode = ImGuizmo::UNIVERSAL;
				break;
			}

			ImGuizmo::Manipulate(camViewPtr, projectionPtr, operationMode, currentGizmoMode, matrixPtr, NULL, useSnap ? &snapValues[0] : NULL, NULL, NULL);

			float matrixTranslation[3], matrixRotation[3], matrixScale[3];
			auto& localTransform = transform.worldToLocal(glmMat);

			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localTransform), matrixTranslation, matrixRotation, matrixScale);

			transform.setLocalPosition(glm::vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]));
			transform.setLocalRotation(glm::vec3(Constants::toRadians* matrixRotation[0], Constants::toRadians* matrixRotation[1], Constants::toRadians* matrixRotation[2]));
			transform.setLocalScale(glm::vec3(matrixScale[0], matrixScale[1], matrixScale[2]));
		}

	}

	if(state.getSelectedEntity() != Entity::EmptyEntity && 
		state.getSelectedEntity().HasComponent<CameraComponent>() && 
		state.getSelectedEntity() == Engine::get()->getContext()->getActiveScene()->getGameCamera())
	{
		// Create a child window

		ImVec2 cameraPreviewSize = ImVec2(300, 200);
		// Placeholder for camera frame

		ImVec2 bottomRightOffset = ImVec2(10, 10);  // Padding from the bottom-right corne

		// Adjust cursor position for the child window
		ImVec2 childPos = ImVec2(
			renderViewWindowSize.x - cameraPreviewSize.x - bottomRightOffset.x,
			renderViewWindowSize.y - cameraPreviewSize.y - bottomRightOffset.y);

		ImGui::SetCursorPos(childPos);
		ImGui::BeginChild("Camera Preview", cameraPreviewSize, true, ImGuiWindowFlags_NoScrollbar);

		ImVec2 contentSize = ImGui::GetContentRegionAvail(); // Get size of the available region


		// Add content to the child window (camera preview)
		ImGui::Text("Camera Preview");
		ImGui::Separator();

		

		auto renderTargetID = Engine::get()->getContext()->getActiveScene()->getGameRenderViewTextureID();
		ImGui::Image(reinterpret_cast<ImTextureID>(renderTargetID), cameraPreviewSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::EndChild();
	}

	ImGui::End();
}

void RenderConsoleWindow()
{
        ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus |
											ImGuiWindowFlags_NoCollapse |
											ImGuiWindowFlags_NoFocusOnAppearing |
											ImGuiWindowFlags_NoTitleBar |
											ImGuiWindowFlags_NoMove);

        std::lock_guard<std::mutex> lock(g_consoleMutex);
        for (const auto& line : g_consoleLog)
        {

            ImGui::TextColored(LogLevelToColor(line.level), line.msg.c_str());
        }
        if (g_scrollConsole)
        {
            ImGui::SetScrollHereY(1.0f);
            g_scrollConsole = false;
        }

        ImGui::End();
}

void DisplayDebugInfoWindow()
{
	//if (displayDebugInfoWindow)
	ImVec2 windowSize(400.f, 400.f);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	ImGui::Begin("Debug Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// --- Basic Debug Info ---
	auto fps = Engine::get()->getSubSystem<System>()->getFPS();
	ImGui::Text("FPS: %.1f", fps);

	auto deltaTime = Engine::get()->getSubSystem<System>()->getDeltaTime() * 1000;
	ImGui::Text("Delta time: %.1f ms", deltaTime);

	auto triangleCount = Engine::get()->getSubSystem<System>()->getTriangleCount();
	ImGui::Text("Triangle count: %u", triangleCount);

	ImGui::Separator();

	// --- Texture List ---
	const auto& debugTextures = DebugHelper::getInstance().getDebugTextures();

	ImGui::Text("Debug Textures:");
	for (const auto& [name, textureResourceWrapper] : debugTextures)
	{
		// Show image button
		ImGui::BeginGroup();
		ImGui::PushID(name.c_str());

		if (ImGui::CollapsingHeader(name.c_str()))
		{
			ImTextureID texID = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(textureResourceWrapper.get()->getID()));
			if (ImGui::ImageButton(texID, ImVec2(100, 100)))
			{
				EditorState::Instance().selectedTextureName = name;
				EditorState::Instance().showTextureDisplayWindow = true;
				ImGui::OpenPopup("Texture Preview");
			}
		}


		ImGui::PopID();
		ImGui::EndGroup();
	}

	ImGui::End();
}


class GUI_Helper : public GuiMenu {
	// Inherited via GuiMenu
	virtual void display() override {

		RenderCommand::clear();

		// Get the total screen size
		int screenWidth = Engine::get()->getWindow()->getWidth();
		int screenHeight = Engine::get()->getWindow()->getHeight();

		


		// Render menu bar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File")) 
				{ // Start of File dropdown
					if (ImGui::MenuItem("Open Project", "Ctrl+O")) {


						const char* lTheSelectFolderName = tinyfd_selectFolderDialog(
							"let us just select a directory", "../../");

						//nfdchar_t* outPath = NULL;
						//nfdresult_t result = NFD_PickFolder(NULL, &outPath);
						if (lTheSelectFolderName)
						{
							Engine::get()->loadProject(lTheSelectFolderName);
							state.selectEntity(Entity::EmptyEntity);
							updateScene();
						}
					}
					if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
						Engine::get()->saveProject();
					}
					if (ImGui::BeginMenu("Import")) {
						if (ImGui::MenuItem("Model")) {
							ShowModelCreatorWindow();
						}
						if (ImGui::MenuItem("Texture")) {
							ShowTextureImportWindow();
						}
						if (ImGui::MenuItem("Animation")) {
							// Action for importing animation
							ShowAnimationImportWindow();
						}
						ImGui::EndMenu();
					}
					if (ImGui::MenuItem("Build", "")) {
						// Path to the Python script
						std::string pythonScriptPath = "../../scripts/build_shipping.py";

			// Determine asset, output, and solution paths
			std::string assetsFolder = Engine::get()->getInitParams().projectDir;
			std::string outputFolder = std::string(SGE_SOLUTION_DIR) + "/../Release";
			std::string solutionDir = std::string(SGE_SOLUTION_DIR);

			// Command to execute the Python script with project, output, and solution paths
			std::string command = "python \"" + pythonScriptPath + "\" \"" + assetsFolder + "\" \"" + outputFolder + "\" \"" + solutionDir + "\"";

			// Run the command
			std::system(command.c_str());

			//std::filesystem::create_directories("../Game/data");
					}
					if (ImGui::MenuItem("Reload config", "")) {
						Engine::get()->reloadEngineConfig();
					}
					ImGui::Separator(); // Optional: Add a separator
					if (ImGui::MenuItem("Quit", "Alt+F4")) {
						Engine::get()->stop();
					}
					ImGui::EndMenu(); // End of File dropdown
				}
				if (ImGui::BeginMenu("Edit")) {
					if (ImGui::BeginMenu("Create")) {
						if (ImGui::MenuItem("Empty Texture")) {
							EditorState::Instance().showTextureCreateWindow = true;
							
						}
						if (ImGui::MenuItem("Shader Override")) {
							EditorState::Instance().showShaderCreateWindow = true;

						}

						if (ImGui::MenuItem("Prefab")) {
							Prefab::import("", {});

						}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Help")) {
					// Help menu items
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
			
			ImGui::EndMainMenuBar();
		}

		ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton);
		static bool dockspace_initialized = false;
		if (!dockspace_initialized)
		{
			dockspace_initialized = true;
			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

			ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
			ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
			ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.3f, nullptr, &dockspace_id);
			ImGuiID dock_id_top = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.05f, nullptr, &dockspace_id);

			if (ImGuiDockNode* topNode = ImGui::DockBuilderGetNode(dock_id_top))
{
				topNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoDockingOverMe;
}

			ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_id_left);
			ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
            ImGui::DockBuilderDockWindow("Asset View", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Simulation Controls", dock_id_top);
            ImGui::DockBuilderDockWindow("View", dockspace_id);
            ImGui::DockBuilderFinish(dockspace_id);
		}

		// Render UI
		RenderSimulationControlView();
		RenderViewWindow();
		RenderSceneHierarchyWindow();
		InspectorWindow::display();
        AssetViewWindow::display();
        RenderConsoleWindow();
		displayTextureCreatorDialog();
		displayShaderCreatorDialog();
		displayMaterialEditDialog();
        //ShowTextureDisplayWindow();

		if (EditorState::Instance().showAssetSelectorWindow) 
		{
			UUID uuid;
			displayAssetSelectDialog(EditorState::Instance().assetSelectType, uuid);
			if (!uuid.empty())
			{
				if (EditorState::Instance().assetSelectCB)
				{
					EditorState::Instance().assetSelectCB(uuid);
				}
			}
		}

		DisplayDebugInfoWindow();
		
	}
};

void setStyleAndColors()
{
	// Todo move to .ini file
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = ImGui::GetStyle().Colors;

	// Backgrounds
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.12f, 0.14f, 1.00f); // #1b1f23
	colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.11f, 0.13f, 0.98f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);

	// Text
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.97f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.55f, 0.60f, 1.00f);

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);

	// Frames, Inputs
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.30f, 0.40f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.27f, 0.33f, 0.45f, 1.00f);

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.53f, 0.90f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.48f, 0.80f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.196f, 0.196f, 0.196f, 1.0f);

	// Buttons (more vivid, professional tone)
	colors[ImGuiCol_Button] = ImVec4(0.18f, 0.22f, 0.30f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.32f, 0.55f, 0.92f, 1.00f); // #4091f7
	colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.48f, 0.84f, 1.00f);

	// Interactive elements
	colors[ImGuiCol_SliderGrab] = ImVec4(0.27f, 0.56f, 0.92f, 1.00f); // softer accent
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.34f, 0.63f, 0.96f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.34f, 0.75f, 1.00f, 1.00f);

	// Headers (collapsing section, tree, etc.)
	colors[ImGuiCol_Header] = ImVec4(0.21f, 0.24f, 0.29f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.27f, 0.53f, 0.90f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.48f, 0.84f, 1.00f);

	// Misc
	colors[ImGuiCol_Separator] = ImVec4(0.29f, 0.31f, 0.34f, 0.60f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.55f, 0.90f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 0.55f, 0.90f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25f, 0.55f, 0.90f, 0.95f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.60f, 0.95f, 0.35f);

	style.WindowPadding = ImVec2(10, 10);
	style.FramePadding = ImVec2(6, 4);
	style.ItemSpacing = ImVec2(10, 6);
	style.IndentSpacing = 20.0f;
	style.WindowRounding = 5.0f;
	style.FrameRounding = 4.0f;
	style.GrabRounding = 4.0f;
	style.ScrollbarRounding = 6.0f;
	style.WindowBorderSize = 1.0f;
}



static bool debugTerrainFlag = false;

class EditorApp : public Application
{
public:
	void start() override
	{

		m_editorRegistry = std::make_shared<SGE_Regsitry>();
		
		ImGui::SetCurrentContext((ImGuiContext * )Engine::get()->getImguiHandler()->getCurrentContext());

        setStyleAndColors();

        NativeScriptsLoader::instance->init();

        Engine::get()->getEventSystem()->pushLayer(uiLayer);

		uiHandler = Engine::get()->getEventSystem()->bindToLayer(uiLayer->name);
		gameHandler = Engine::get()->getEventSystem()->bindToLayer("GameLayer");

		auto scene = Engine::get()->getContext()->getActiveScene();

		// store Default scene camera
		g_primaryCamera = scene->getGameCamera();

		// set Editor camera as active camera
		auto editorCamera = m_editorRegistry->createEntity("Editor Camera");
		editorCamera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)4 / 3, 0.1f, 3000.0f));
		editorCamera.addComponent<NativeScriptComponent>().bind<EditorCamera>();
		auto& nsc = editorCamera.getComponent<NativeScriptComponent>();
		nsc.script->eventHandler = Engine::get()->getEventSystem()->bindToLayer(uiLayer->name);
		//Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(editorCamera);

		nsc.script->onCreate();

		g_editorCamera = editorCamera;

		Engine::get()->getInput()->getKeyboard()->onKeyPressed(gameHandler, SDL_SCANCODE_ESCAPE, [](SDL_Event e) { stopSimulation(); });
		
		Engine::get()->getInput()->getKeyboard()->onKeyReleased(uiHandler, SDL_SCANCODE_X, [](SDL_Event e) {
			debugTerrainFlag = true;
			});

		//g_previewWindowID = Engine::get()->getContext()->getActiveScene()->addRenderView(0, 0, 300, 200, g_primaryCamera);
		//g_previewWindowID = Engine::get()->getContext()->getActiveScene()->addRenderView("Game Preview", 0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight(), g_primaryCamera);
		Engine::get()->getContext()->getActiveScene()->addRenderView("Editor View", 0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight(), editorCamera);
		g_previewWindowID = Engine::get()->getContext()->getActiveScene()->getGameRenderViewFrameBufferID();
		//Engine::get()->getContext()->getActiveScene()->setActiveRenderView("Editor View");

		updateScene();

		auto gui = new GUI_Helper();
		Engine::get()->getImguiHandler()->addGUI(gui);

		

		

		Texture::TextureImportSettings settings;
		icons["mesh"] = Texture::loadTransient(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-cube-100.png", settings);
		icons["texture"] = Texture::loadTransient(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-image-100.png", settings);
		icons["animation"] = Texture::loadTransient(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-skeleton-100.png", settings);
		icons["shader"] = Texture::loadTransient(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-code-100.png", settings);
		icons["folder"] = Texture::loadTransient(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-folder-100.png", settings);
		icons["prefab"] = Texture::loadTransient(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-blueprint-100.png", settings);
		icons["material"] = Texture::loadTransient(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-material-100.png", settings);
	}

	void update(float deltaTime) override
	{
		g_editorCamera.getComponent<NativeScriptComponent>().script->onUpdate(deltaTime);
		g_editorCamera.getComponent<Transformation>().update();
	}
	std::shared_ptr<SGE_Regsitry> m_editorRegistry;
	
	EventHandler uiHandler;
	EventHandler gameHandler;
};

Application* CreateApplication()
{
	return new EditorApp();
}