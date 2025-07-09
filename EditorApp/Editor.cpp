#include "Menu.h"
#include "EntryPoint.h"
#include "sge.h"

#include "Window.h"
#include "commdlg.h"

#include <filesystem>

#include "ImGuizmo.h"
#include "imgui_internal.h"

#include "tinyfiledialogs.h"

#include "NativeScriptsLoader.h"
#include "UIEventLayer.h"

#include "EditorCamera.h"
#include "EditorState.h"

#include <imgui_stdlib.h>

#define BEGIN_IMGUI_TABLE(name) \
    if (ImGui::BeginTable(name, 2, ImGuiTableFlags_None)) { \
        ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 0.4f); \
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.6f);

#define END_IMGUI_TABLE() ImGui::EndTable(); };

static const std::string SGE_EDITOR_APP_ROOT = "../../EditorApp/Resources";

static std::unordered_map<std::string, Resource<Texture>> icons;

std::shared_ptr<EventLayer> uiLayer = std::make_shared<UIEventLayer>();

static std::string getAssetTypeAsStr(AssetType aType)
{
	if (aType == AssetType::MESH) return "Mesh";
	if (aType == AssetType::TEXTURE) return "Texture";
	if (aType == AssetType::SHADER) return "Shader";
	if (aType == AssetType::ANIMATION) return "Animation";
	return "N/A";
}

namespace fs = std::filesystem;

static bool ShowLightCreatorWindow = false;
static bool showModelInspectorWindow = false;
static bool showPrimitiveCreatorWindow = false;
static bool showMeshSelector = false;
static bool showAnimationSelector = false;
static bool selectedEntityRename = false;
static bool showScriptSelector = false;
static bool showShaderSelector = false;
static bool showSamplerEditWindow = false;
static bool showTextureCreateWindow = false;
static bool showShaderCreateWindow = false;

static bool startButtonPressed = false;

static std::string selectedTextureName;
static bool showTextureDisplayWindow = false;

Entity g_primaryCamera;
Entity g_editorCamera;

uint32_t g_previewWindowID = 0;

std::function<void(std::string uuid)> assetTextureSelectCB;
Resource<Texture> selectedAssetTexture;

static std::shared_ptr<TextureSampler> g_selectedSampler;
static std::shared_ptr<TextureSampler> g_previousSampler;

static void addTextureEditWidget(std::shared_ptr<Material> mat, const std::string& name, Texture::Type ttype);
void AddColoredLabel(const char* label);
static void displayTransformation(Transformation& transform, bool& isChanged);
static void displaySelectMeshWindow();

struct EntityState
{
	Entity e;

	Resource<Shader> shader;

	std::vector<std::string> animationRenameBuffers{};

	std::string renameBuffer;

	EntityState(Entity e)
		: e(e)
	{
	}

	void update()
	{
		if (e.HasComponent<Animator>())
		{
			auto& animator = e.getComponent<Animator>();
			auto animations = animator.getAllAnimations();
			animationRenameBuffers.clear();
			for (const auto& [name, anim] : animations) {
				animationRenameBuffers.push_back(name);
			}
		}

		if (e.HasComponent<ObjectComponent>())
		{
			auto& obj = e.getComponent<ObjectComponent>();
			renameBuffer = obj.name;
		}
		

	}
};



class EntityStates
{
public:
	void selectEntity(Entity e)
	{
		m_selectedEntity = e;
		Engine::get()->getSubSystem<ObjectPicker>()->setSelectedObject(m_selectedEntity.handlerID());

		if (e == Entity::EmptyEntity)
		{
			return;
		}

		std::shared_ptr<EntityState> eState = std::make_shared<EntityState>(e);
		eState->update();

		m_states[e.handlerID()] = eState;
	}

	EntityState& getCurrentEntityState()
	{
		auto iter = m_states.find(m_selectedEntity.handlerID());
		if (iter == m_states.end())
		{
			auto eState = std::make_shared<EntityState>(m_selectedEntity);
			eState->update();
			m_states[m_selectedEntity.handlerID()] = eState;
			return *eState.get();
		}

		return *iter->second.get();
	}

	Entity getSelectedEntity() const
	{
		return m_selectedEntity;
	}

private:
	std::unordered_map<entity_id, std::shared_ptr<EntityState>> m_states;
	Entity m_selectedEntity = Entity::EmptyEntity;
};

static EntityStates state;





static void stopSimulation()
{
	startButtonPressed = false; // Toggle the state
	Engine::get()->getContext()->getActiveScene()->stopSimulation();

	Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(g_editorCamera);

	uiLayer->setEnabled(true);
	static_cast<EditorCamera*>(g_editorCamera.getComponent<NativeScriptComponent>().script.get())->unlock();
}

static void startsimulation()
{
	startButtonPressed = true; // Toggle the state
	Engine::get()->getContext()->getActiveScene()->startSimulation();

	Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(g_primaryCamera);

	uiLayer->setEnabled(false);
	state.selectEntity(Entity::EmptyEntity);
	static_cast<EditorCamera*>(g_editorCamera.getComponent<NativeScriptComponent>().script.get())->lock();

}

static void displayWindowHeader(const std::string& name)
{
	ImVec2 region = ImGui::GetContentRegionAvail();
	float headerHeight = 24.0f;

	// Header style
	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));
	ImGui::BeginChild(name.c_str(), ImVec2(region.x, headerHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	// Vertical center calculation
	float textHeight = ImGui::GetFontSize();
	float textY = (headerHeight - textHeight) * 0.5f;

	// Horizontal padding
	float paddingLeft = 8.0f;

	// Set cursor position inside child window
	ImGui::SetCursorPos(ImVec2(paddingLeft, textY));
	ImGui::TextUnformatted(name.c_str());

	ImGui::EndChild();
	ImGui::PopStyleColor();
}

template<typename T> 
static void displayComponent(const std::string& componentName, std::function<void(T&)> func)
{
	if (state.getSelectedEntity().HasComponent<T>())
	{
		ImVec2 startPos = ImGui::GetCursorScreenPos();
		ImVec2 startPosCursor = ImGui::GetCursorPos(); // Capture the initial cursor position

		AddColoredLabel(componentName.c_str());
		auto& component = state.getSelectedEntity().getComponent<T>();

		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImVec2 windowSize = ImGui::GetWindowSize();

		if (!std::is_same<T, Transformation>::value)
		{
			ImGui::SetCursorPos(ImVec2(windowSize.x - 24.0f, cursorPos.y - ImGui::GetTextLineHeightWithSpacing() - 7.0f));
			ImGui::PushID(componentName.c_str());
			if (ImGui::Button("X")) {
				state.getSelectedEntity().RemoveComponent<T>();
				ImGui::EndGroup();
				updateScene();
				return;
			}
			ImGui::PopID();
		}

		ImGui::Indent(5); // Indent by 10 pixels
		func(component);
		ImGui::Unindent(5); // Remove the indent

		ImVec2 endPosCursor = ImGui::GetCursorPos(); // Capture the cursor position before adding the separator
		ImVec2 endPos = ImVec2(startPos.x + ImGui::GetContentRegionAvail().x, startPos.y + (endPosCursor.y - startPosCursor.y));

		ImGui::Dummy(ImVec2(0, 4));

		ImGui::Separator();

		ImGui::Dummy(ImVec2(0, 4));

		// Adjust the rectangle to the correct end position
		//ImGui::GetWindowDrawList()->AddRect(startPos, endPos, ImGui::GetColorU32(ImGuiCol_Header), 0.f, 0, 2.f);
	}
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

void RenderSimulationControlView(float width, float height)
{
	float windowWidth = width * 0.7f - 10;
	float windowHeight = 35;
	float startX = width * 0.15f + 10; // Add a gap of 10 pixels
	ImGui::SetNextWindowPos(ImVec2(startX, 25)); // Adjust vertical position to make space for the menu bar
	ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

	ImGui::Begin("Centered Buttons", nullptr, ImGuiWindowFlags_NoDecoration);

	// Center align the buttons
	ImGui::SetCursorPosX((windowWidth - 100) * 0.5f);



	// Draw the button based on the current state
	if (startButtonPressed) {
		if (ImGui::Button("STOP", ImVec2(70, 0))) {
			// Handle stop button click
			stopSimulation();
		}
	}
	else {
		if (ImGui::Button("START", ImVec2(70, 0))) {
			// Handle start button click
			startsimulation();
		}
	}

	ImGui::End(); // End the window
}

static void addTableRow(const std::string& rowName, std::function<void(std::string id)> func)
{
	ImGui::TableNextRow();

	// Key: stick to left (default)
	ImGui::TableSetColumnIndex(0);
	ImGui::TextUnformatted(rowName.c_str());

	// Value: right-align the DragFloat3
	ImGui::TableSetColumnIndex(1);

	const float fullWidth = ImGui::GetColumnWidth();
	const float itemWidth = ImGui::CalcItemWidth(); // or CalcItemWidth(), or a fixed value
	float cursorX = ImGui::GetCursorPosX() + fullWidth - itemWidth;

	ImGui::SetCursorPosX(cursorX);
	ImGui::SetNextItemWidth(itemWidth);
	std::string id = "##" + rowName;
	func(id);
}

static void addTableRowExt(const std::string& rowName, 
	std::function<void(std::string id)> funcKey, 
	std::function<void(std::string id)> funcValue)
{
	ImGui::TableNextRow();

	// Key: stick to left (default)
	ImGui::TableSetColumnIndex(0);
	funcKey(rowName.c_str());

	// Value: right-align the DragFloat3
	ImGui::TableSetColumnIndex(1);

	const float fullWidth = ImGui::GetColumnWidth();
	const float itemWidth = ImGui::CalcItemWidth(); // or CalcItemWidth(), or a fixed value
	float cursorX = ImGui::GetCursorPosX() + fullWidth - itemWidth;

	ImGui::SetCursorPosX(cursorX);
	ImGui::SetNextItemWidth(itemWidth);
	std::string id = "##" + rowName;
	funcValue(id);
}                                            

static void displayTransformation(Transformation& transform, bool& isChanged)
{
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	auto& localTransform = transform.getLocalTransformation();
	ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localTransform), matrixTranslation, matrixRotation, matrixScale);

	BEGIN_IMGUI_TABLE("Transform");

	addTableRow("Position", [&](std::string id) {
		if (ImGui::DragFloat3(id.c_str(), matrixTranslation, .1f)) {
			transform.setLocalPosition(glm::vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]));
			isChanged = true;
		}
	});

	addTableRow("Rotation", [&](std::string id) {
		if (ImGui::DragFloat3(id.c_str(), matrixRotation), .001f, 0.f, 180.f) {
			transform.setLocalRotation(glm::radians(glm::vec3(matrixRotation[0], matrixRotation[1], matrixRotation[2])));
			isChanged = true;
		}
	});

	addTableRow("Scale", [&](std::string id) {
		if (ImGui::DragFloat3(id.c_str(), matrixScale, .1f)) {
			transform.setLocalScale(glm::vec3(matrixScale[0], matrixScale[1], matrixScale[2]));
			isChanged = true;
		}
	});

	END_IMGUI_TABLE();

}

static void displayAssetTextureSelectPopup()
{
	if (ImGui::BeginPopup("EditTexturePopup")) {

		auto assets = Engine::get()->getSubSystem<Assets>();

		ImGui::Text("Available Textures:");
		ImGui::Separator();

		static int selectedTextureIndex = -1;

		auto& textureList = assets->getAllAssetsOfType(AssetType::TEXTURE);

		if (selectedTextureIndex != -1)
		{
			Resource<Texture> displayTexture(textureList.at(selectedTextureIndex).uuid);
			ImVec2 imageSize(150, 150);
			ImGui::Image(reinterpret_cast<ImTextureID>(displayTexture.get()->getID()), imageSize, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		}

		ImGui::Separator();

		for (int i = 0; i < textureList.size(); i++)
		{
			bool isSelected = (selectedTextureIndex == i);
			if (isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Change background color
			}
			if (!isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Default color
			}

			if (ImGui::Selectable(textureList[i].name.c_str(), false, ImGuiSelectableFlags_DontClosePopups))
			{
				selectedTextureIndex = i;
			}

			ImGui::PopStyleColor();
		}

		ImGui::Separator();

		if (ImGui::Button("OK")) {
			if (selectedTextureIndex >= 0 && selectedTextureIndex < textureList.size())
			{
				assetTextureSelectCB(textureList[selectedTextureIndex].uuid);

			}
			ImGui::CloseCurrentPopup();
			selectedTextureIndex = -1;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
			selectedTextureIndex = -1;
		}

		

		ImGui::EndPopup();
	}

}

static void displaySelectMeshWindow(std::string& uuid)
{
	if (showMeshSelector) 
	{
		ImGui::Begin("Select Mesh", &showMeshSelector, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Available Meshes:");
		ImGui::Separator();

		static int selectedMeshIndex = -1;

		auto& meshList = Engine::get()->getSubSystem<Assets>()->getAllAssetsOfType(AssetType::MESH); // todo fix

		for (int i = 0; i < meshList.size(); i++) 
		{
			bool isSelected = (selectedMeshIndex == i);
			if (isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Change background color
			}
			if (!isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Default color
			}

			if (ImGui::Selectable(meshList[i].name.c_str()))
			{
				selectedMeshIndex = i;
			}

			ImGui::PopStyleColor();
		}

		ImGui::Separator();

		if (ImGui::Button("OK")) {
			if (selectedMeshIndex >= 0 && selectedMeshIndex < meshList.size()) 
			{
				uuid = meshList[selectedMeshIndex].uuid;
				
			}
			showMeshSelector = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			showMeshSelector = false;
		}

		ImGui::End();
	}
}

static void displaySelectAnimationWindow(std::string& uuid)
{
	if (showAnimationSelector)
	{
		ImGui::Begin("Select Animation", &showAnimationSelector, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Available Animations:");
		ImGui::Separator();

		static int selectedAnimationIndex = -1;

		auto& animationList = Engine::get()->getSubSystem<Assets>()->getAllAssetsOfType(AssetType::ANIMATION);

		for (int i = 0; i < animationList.size(); i++)
		{
			bool isSelected = (selectedAnimationIndex == i);
			if (isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Change background color
			}
			if (!isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Default color
			}

			if (ImGui::Selectable(animationList[i].name.c_str()))
			{
				selectedAnimationIndex = i;
			}

			ImGui::PopStyleColor();
		}

		ImGui::Separator();

		if (ImGui::Button("OK")) {
			if (selectedAnimationIndex >= 0 && selectedAnimationIndex < animationList.size())
			{
				uuid = animationList[selectedAnimationIndex].uuid;

			}
			showAnimationSelector = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			showAnimationSelector = false;
		}

		ImGui::End();
	}
}

static void displaySelectScriptWindow(std::string& scriptName)
{
	if (showScriptSelector)
	{
		ImGui::Begin("Select Script", &showScriptSelector, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Available Scripts:");
		ImGui::Separator();

		static int selectedScriptIndex = -1;

		std::vector<std::string> scriptNamesList;
		NativeScriptsLoader::instance->getAllScripts(scriptNamesList);

		for (int i = 0; i < scriptNamesList.size(); i++)
		{
			bool isSelected = (selectedScriptIndex == i);

			if (ImGui::Selectable(scriptNamesList[i].c_str(), &isSelected))
			{
				selectedScriptIndex = i;
			}
		}

		ImGui::Separator();

		if (ImGui::Button("OK")) 
		{
			if (selectedScriptIndex >= 0 && selectedScriptIndex < scriptNamesList.size())
			{
				scriptName = scriptNamesList[selectedScriptIndex];

			}
			showScriptSelector = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) 
		{
			showScriptSelector = false;
		}

		ImGui::End();
	}
}

static void displaySelectShaderWindow(std::string& uuid)
{
	if (showShaderSelector)
	{
		ImGui::Begin("Select Shader", &showShaderSelector, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Available Shaders:");
		ImGui::Separator();

		static int selectedShaderIndex = -1;

		auto& shaderList = Engine::get()->getSubSystem<Assets>()->getAllAssetsOfType(AssetType::SHADER);


		for (int i = 0; i < shaderList.size(); i++)
		{
			bool isSelected = (selectedShaderIndex == i);

			if (ImGui::Selectable(shaderList[i].name.c_str(), &isSelected))
			{
				selectedShaderIndex = i;
			}
		}

		ImGui::Separator();

		if (ImGui::Button("OK"))
		{
			if (selectedShaderIndex >= 0 && selectedShaderIndex < shaderList.size())
			{
				uuid = shaderList[selectedShaderIndex].uuid;

			}
			showShaderSelector = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			showShaderSelector = false;
		}

		ImGui::End();
	}
}


// Define a structure to represent an object in the scene hierarchy
struct SceneObject {
	std::string name;
	Entity e;
	// Add any other properties as needed
};

// Define a vector to store scene objects
std::vector<SceneObject> sceneObjects
{};

void updateScene()
{
	sceneObjects.clear();
	for (auto&& [entity, obj] : Engine::get()->getContext()->getActiveScene()->getRegistry().get().view<ObjectComponent>().each())
	{
		sceneObjects.emplace_back(SceneObject{ obj.name, obj.e });
	}
}

auto style = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;

void AddColoredLabel(const char* label) 
{
	// Draw a blue background using ImGuiCol_Header color
	ImVec2 startPos = ImGui::GetCursorScreenPos();
	ImVec2 endPos = ImVec2(startPos.x + ImGui::GetContentRegionAvail().x, startPos.y + ImGui::GetTextLineHeightWithSpacing() + 2);
	ImGui::GetWindowDrawList()->AddRectFilled(startPos, endPos, ImGui::GetColorU32(ImGuiCol_Header));

	// Calculate the vertical offset to center the text within the rectangle
	float offsetY = (ImGui::GetTextLineHeightWithSpacing() - ImGui::GetFrameHeight()) * 0.5f;

	// Calculate padding values
	float paddingX = 5.0f;
	float paddingY = 2.0f;

	// Adjust the text position to center it vertically and add padding
	ImVec2 textPos = ImVec2(startPos.x + paddingX, startPos.y + offsetY + paddingY);

	// Render the label text
	ImGui::SetCursorScreenPos(textPos);
	ImGui::TextUnformatted(label);

	ImGui::Dummy(ImVec2(0.0f, 2.0f)); // Add a vertical gap
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

enum class LightType {
	DirectionalLight = 0,
	PointLight = 1
};

void LightCreatorWindow()
{
	if (ShowLightCreatorWindow)
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

			ShowLightCreatorWindow = false;

			updateScene();

			//logInfo("Added light successfully.");
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ShowLightCreatorWindow = false;
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

void ShowTextureDisplayWindow()
{
	if (showTextureDisplayWindow)
	{
		ImGui::OpenPopup("Texture Preview");
		showTextureDisplayWindow = false;
	}
	if (ImGui::BeginPopupModal("Texture Preview", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		const auto& selectedRes = DebugHelper::getInstance().getDebugTextures().at(selectedTextureName);
		Texture* tex = selectedRes.get();

		if (tex)
		{
			ImTextureID texID = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex->getID()));
			ImVec2 texSize(512, 512); // Preview size (can be dynamic)

			ImGui::Text("%s", selectedTextureName.c_str());
			ImGui::Image(texID, texSize);
		}

		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
			showTextureDisplayWindow = false;
		}

		ImGui::EndPopup();
	}
}

void ShowTextureCreatorWindow()
{
	if (showTextureCreateWindow)
	{
		ImGui::OpenPopup("CreateEmptyTexture");
		showTextureCreateWindow = false;
	}
	if (ImGui::BeginPopupModal("CreateEmptyTexture", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static int width = 512;
		static int height = 512;
		static char textureName[256] = "NewTexture";

		ImGui::InputInt("Width", &width);
		ImGui::InputInt("Height", &height);
		ImGui::InputText("Name", textureName, IM_ARRAYSIZE(textureName));

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			auto texture = Texture::createEmptyTexture(width, height);
			Texture::addTexture2D(textureName, texture);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void ShowShaderCreatorWindow()
{
	if (showShaderCreateWindow)
	{
		ImGui::OpenPopup("CreateShader");
		showShaderCreateWindow = false;
	}
	if (ImGui::BeginPopupModal("CreateShader", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char shaderName[256] = "New Shader";
		static char filepath[256] = "";
		static int shaderOverrideType = 0;

		// Shader Name
		ImGui::Text("Name");
		ImGui::InputText("##ShaderName", shaderName, IM_ARRAYSIZE(shaderName), ImGuiInputTextFlags_EnterReturnsTrue);

		// Shader File Path
		ImGui::Text("Filepath");
		ImGui::InputText("##ShaderFilePath", filepath, IM_ARRAYSIZE(filepath), ImGuiInputTextFlags_EnterReturnsTrue);

		// Override Type Drop-down
		const char* overrideTypes[] = { "PBR Basic Shader", "Pixel Shader" };
		ImGui::Text("Override Type");
		ImGui::Combo("##ShaderOverrideType", (int*)&shaderOverrideType, overrideTypes, IM_ARRAYSIZE(overrideTypes));

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			auto& shader = Shader::createOverrideShader(shaderName, filepath, (ShaderOverride)shaderOverrideType);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
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

	Texture::importTexture2D(texturePathBuffer.c_str());

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

	Engine::get()->getSubSystem<AnimationLoader>()->import(animationPathBuffer.c_str());

	animationPathBuffer.clear();
}

void ShowModelCreatorWindow()
{
	static ImGuiTextBuffer modelPathBuffer;

	const char* filepath = tinyfd_openFileDialog(
		"Select an asset to load",
		"",
		5,
		g_supportedFormats,
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

	auto modelInfo = Engine::get()->getSubSystem<ModelImporter>()->import(modelPathBuffer.c_str());
	entity.addComponent<MeshComponent>().mesh = modelInfo.mesh;

	auto& materialComponent = entity.addComponent<MaterialComponent>();
	for(auto& [idx, m] : modelInfo.materials)
	{
		materialComponent.setMaterial(idx, m);
	}

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
				selectedEntityRename = true;
			}

			if (e.HasComponent<CameraComponent>())
			{
				if (ImGui::MenuItem("Set as Primary Camera"))
				{
					auto scene = Engine::get()->getContext()->getActiveScene();
					scene->setPrimaryCamera(e);
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
	if (selectedEntityRename && state.getSelectedEntity() == e)
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
			selectedEntityRename = false;
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

	static int nonLeafTreeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;
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
		if (transform.getParent().valid()) continue;

		ImGui::PushID(i); // Push a unique ID to avoid ImGui ID conflicts
		
		displayEntity(sceneObject.e);

		ImGui::PopID();
	}

	ImGui::EndChild(); // End background drop zone
}

void RenderSceneHierarchyWindow(float width, float height)
{
	float windowWidth = width * 0.15f;
	ImVec2 windowPos(5, 25); // Adjust vertical position to make space for the menu bar
	ImVec2 windowSize(windowWidth, height * 0.7f);
	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(windowSize);
	ImGui::Begin("Scene Hierarchy", nullptr, style);

	displayWindowHeader("Scene Hierarchy");

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
				Entity e = Skybox::createSkybox(SGE_ROOT_DIR + "Resources/Engine/Textures/sunflowers_puresky_4k.hdr", Skybox::TexType::EQUIRECTANGULAR);
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

	// Calculate the size of the list box accounting for padding
	ImVec2 listBoxSize(windowSize.x - 20, windowSize.y - 70);

	ImGui::SetNextWindowSize(listBoxSize);

	// Render list view
	if (ImGui::BeginListBox("##Objects", listBoxSize)) 
	{
		// Iterate through each scene object and render it as a selectable item in the list
		displaySceneObjects();
		ImGui::EndListBox();
	}

	ImGui::End();
}

void RenderViewWindow(float width, float height) 
{
	ImVec2 renderViewWindowSize(width * 0.7f - 10, height * 0.7f - 40);
	float startX = width * 0.15f + 10; // Add a gap of 10 pixels
	ImGui::SetNextWindowPos(ImVec2(startX, 65)); // Adjust vertical position to make space for the menu bar
	ImGui::SetNextWindowSize(ImVec2(renderViewWindowSize.x, renderViewWindowSize.y));
	ImGui::Begin("View", nullptr, style | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDecoration);
	
	// Display the texture
	ImVec2 imageSize(renderViewWindowSize.x, renderViewWindowSize.y);
	ImGui::Image(reinterpret_cast<ImTextureID>(Engine::get()->getContext()->getActiveScene()->getRenderTargetTextureID(0)), imageSize, ImVec2(0, 1), ImVec2(1, 0));

	ImVec2 mousePos = ImGui::GetMousePos();
	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 windowSize = ImGui::GetWindowSize();

	// Check if the mouse is within the window bounds
	if (mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + windowSize.x &&
		mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + windowSize.y)
	{
		EditorState::Instance().isMouseInSceneView = true;
	}
	else
	{
		EditorState::Instance().isMouseInSceneView = false;
	}

	if (!Engine::get()->getContext()->getActiveScene()->isSimulationActive() && 
		!ImGui::IsPopupOpen(1, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
	{

		if (!ImGuizmo::IsUsing() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 windowSize = ImGui::GetWindowSize();

			// Check if the mouse is within the window bounds
			if (mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + windowSize.x &&
				mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + windowSize.y)
			{
				// We alter the mouse position from small window into full screen (the renderered object pick texture)
				int alteredX = (mousePos.x - startX) / renderViewWindowSize.x * Engine::get()->getWindow()->getWidth();
				int alteredY = (mousePos.y - 65) / renderViewWindowSize.y * Engine::get()->getWindow()->getHeight();
				int selectedID = Engine::get()->getSubSystem<ObjectPicker>()->pickObject(alteredX, alteredY);

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

		
		// Define the size and position of the inner window
		float innerWindowWidth = renderViewWindowSize.x - 10;
		float innerWindowHeight = 35.0f;
		ImGui::SetNextWindowPos(ImVec2(startX + 7, 72)); // Adjust position as needed
		ImGui::SetNextWindowSize(ImVec2(innerWindowWidth, innerWindowHeight)); // Adjust size as needed

		// Transformation mode enum and current mode variable
		enum TransformMode { TRANSLATE, ROTATE, SCALE, UNIVERSAL };
		static TransformMode currentMode = TRANSLATE;

		// Gizmo mode variable
		static ImGuizmo::MODE currentGizmoMode = ImGuizmo::LOCAL;

		// Snap options
		static bool useSnap = false;
		static float snapValues[3] = { 1.0f, 1.0f, 1.0f };

		

		if (ImGui::BeginChild("TransformWindow", ImVec2(innerWindowWidth, innerWindowHeight), true))
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

			auto camView = Engine::get()->getContext()->getActiveScene()->getActiveCameraView();
			const float* camViewPtr = glm::value_ptr(camView);

			auto projection = Engine::get()->getContext()->getActiveScene()->getProjection();
			const float* projectionPtr = glm::value_ptr(projection);

			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, renderViewWindowSize.x, renderViewWindowSize.y);

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

		

		auto renderTargetID = Engine::get()->getContext()->getActiveScene()->getRenderTargetTextureID(g_previewWindowID);
		ImGui::Image(reinterpret_cast<ImTextureID>(renderTargetID), cameraPreviewSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::EndChild();
	}

	ImGui::End();
}

static const char* rigidyBodyTypesStrList[]{ 
	"Static",
	"Dynamic",
	"Kinematic"
};

static const char* renderTechniqueStrList[]{
	"Forward",
	"Defererred"
};

static const char* layerMaskList[]{
	"Layer Mask 0",
	"Layer Mask 1",
	"Layer Mask 2",
	"Layer Mask 3",
	"Layer Mask 4",
	"Layer Mask 5",
	"Layer Mask 6",
	"Layer Mask 7",
	"Layer Mask 8",
	"Layer Mask 9",
	"Layer Mask 10",
	"Layer Mask 11",
	"Layer Mask 12",
	"Layer Mask 13",
	"Layer Mask 14",
	"Layer Mask 15",
};

static void displayChannelSelectWidget(int*& currentChannel)
{
	static const char* channelMaskOptions[] = { "None", "R", "G", "B", "A" };
	if (ImGui::BeginCombo("##channelMask", channelMaskOptions[*currentChannel])) // Label for the combo box
	{
		for (int i = 0; i < IM_ARRAYSIZE(channelMaskOptions); i++)
		{
			bool isSelected = (*currentChannel == i);
			if (ImGui::Selectable(channelMaskOptions[i], isSelected))
			{
				*currentChannel = i; // Update selected index
			}

			if (isSelected)
				ImGui::SetItemDefaultFocus(); // Set focus to the current item
		}
		ImGui::EndCombo();
	}
}

static void addTextureEditWidget(int textureID, ImVec2 size, std::function<void(std::string uuid)> callback)
{
	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(textureID), size))
	{
		ImGui::OpenPopup("EditTexturePopup");
		assetTextureSelectCB = callback;
	}

	displayAssetTextureSelectPopup();
}

static void addTextureEditWidget(Resource<Texture> texture, ImVec2 size, std::function<void(std::string uuid)> callback)
{
	int texID = 0;
	if (!texture.isEmpty())
	{
		texID = texture.get()->getID();
	}

	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(texID), size))
	{
		ImGui::OpenPopup("EditTexturePopup");
		assetTextureSelectCB = callback;
	}

	displayAssetTextureSelectPopup();
}

static void addTextureEditWidget(std::shared_ptr<Material> mat, const std::string& name, Texture::TextureType ttype)
{
	Resource<Texture> tex = Resource<Texture>::empty;
	if (mat->hasTexture(ttype))
	{
		tex = mat->getSampler(ttype)->texture;
	}

	addTextureEditWidget(tex, { 20, 20 }, [=](std::string uuid) {
		mat->setTexture(ttype, Resource<Texture>(uuid));
	});

	ImGui::SameLine();

	ImGui::Text(name.c_str());
}

static void addSamplerEditWidget(std::shared_ptr<Material> mat, ImVec2 size, const std::string& name, Texture::TextureType ttype)
{
	ImGui::PushID(name.c_str());

	int texID = 0;
	auto sampler = mat->getSampler(ttype);

	if (!sampler->texture.isEmpty())
	{
		texID = sampler->texture.get()->getID();
	}

	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(texID), size)) 
	{
		ImGui::OpenPopup("EditSamplerPopup");
		g_selectedSampler = sampler;
		g_previousSampler = std::make_shared<TextureSampler>(*sampler.get());		
	}

	if (ImGui::BeginPopup("EditSamplerPopup"))
	{
		if (!g_selectedSampler)
		{
			logError("Selected sampler cannot be null.");
			ImGui::EndPopup();
			return;
		}
		auto assets = Engine::get()->getSubSystem<Assets>();

		ImGui::Text("Texture");
		addTextureEditWidget(g_selectedSampler->texture, ImVec2{150, 150}, [=](std::string uuid) {
			g_selectedSampler->texture = Resource<Texture>(uuid);
			});

		ImGui::Spacing();

		static int* currentChannelMask[4];
		
		currentChannelMask[0] = &g_selectedSampler->channelMaskR;
		currentChannelMask[1] = &g_selectedSampler->channelMaskG;
		currentChannelMask[2] = &g_selectedSampler->channelMaskB;
		currentChannelMask[3] = &g_selectedSampler->channelMaskA;

		for (int i = 0; i < g_selectedSampler->channelCount; i++)
		{
			ImGui::PushID(&currentChannelMask[i]);
			displayChannelSelectWidget(currentChannelMask[i]);
			ImGui::PopID();
		}

		ImGui::Spacing();

		ImGui::DragFloat("xoffset", &g_selectedSampler->xOffset, .1f);
		ImGui::DragFloat("yoffset", &g_selectedSampler->yOffset, .1f);

		ImGui::Spacing();

		ImGui::DragFloat("xScale", &g_selectedSampler->xScale, .1f);
		ImGui::DragFloat("yScale", &g_selectedSampler->yScale, .1f);

		ImGui::Separator();

		if (ImGui::Button("OK")) 
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) 
		{
			mat->setSampler(ttype, g_previousSampler);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::SameLine();
	ImGui::Text(name.c_str());

	ImGui::PopID();
	
}

void rightAlignedText(const std::string& text) {
	float textWidth = ImGui::CalcTextSize(text.c_str()).x;
	float fullWidth = ImGui::GetColumnWidth();
	float padding = ImGui::GetStyle().ItemSpacing.x;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + fullWidth - textWidth - padding);
	ImGui::TextUnformatted(text.c_str());
}

static bool showWindow = true;

void RenderInspectorWindow(float width, float height) 
{
	auto assets = Engine::get()->getSubSystem<Assets>();

	float windowWidth = width * 0.15f - 5;
	float startX = width * 0.85f + 5; // Add a gap of 5 pixels
	ImGui::SetNextWindowPos(ImVec2(startX, 25)); // Adjust vertical position to make space for the menu bar
	ImGui::SetNextWindowSize(ImVec2(windowWidth - 5, height * 0.7f));
	ImGui::Begin("Inspector", &showWindow, style | ImGuiWindowFlags_NoScrollbar);

	displayWindowHeader("Inspector");

	if (state.getSelectedEntity() != Entity::EmptyEntity)
	{
		displayComponent<Transformation>("Transformation", [](Transformation& transform) {
			bool isChanged = false;
			displayTransformation(transform, isChanged);
		});

		displayComponent<PhysicsComponent>("Physics", [](PhysicsComponent& rBody) {
			ImGui::Combo("##Type", (int*)&rBody.type, rigidyBodyTypesStrList, IM_ARRAYSIZE(rigidyBodyTypesStrList));
			ImGui::InputFloat("Mass", &rBody.mass);

			ImGui::LabelText("", "Linear Lock");
			ImGui::PushID("LinearX");
			ImGui::Checkbox("X", &rBody.isLockedLinearX);
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::PushID("LinearY");
			ImGui::Checkbox("Y", &rBody.isLockedLinearY);
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::PushID("LinearZ");
			ImGui::Checkbox("Z", &rBody.isLockedLinearZ);
			ImGui::PopID();

			ImGui::LabelText("", "Angular Lock");
			ImGui::PushID("AngularX");
			ImGui::Checkbox("X", &rBody.isLockedAngularX);
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::PushID("AngularY");
			ImGui::Checkbox("Y", &rBody.isLockedAngularY);
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::PushID("AngularZ");
			ImGui::Checkbox("Z", &rBody.isLockedAngularZ);
			ImGui::PopID();

			

			static const char* colliderTypeNames[] = {
				"None", "Box", "Sphere", "Terrain", "Mesh", "Capsule"
			};

			if (ImGui::Combo("Collider Type", (int*)&rBody.colliderType, colliderTypeNames, IM_ARRAYSIZE(colliderTypeNames))) {
				switch (rBody.colliderType) {
				case ColliderType::NONE: rBody.collider = 0; break;
				case ColliderType::BOX: rBody.collider = std::make_shared<CollisionBox>(); break;
				case ColliderType::SPHERE: rBody.collider = std::make_shared<CollisionSphere>(); break;
				case ColliderType::TERRAIN: rBody.collider = std::make_shared<CollisionTerrain>(); break;
				case ColliderType::MESH: rBody.collider = std::make_shared<CollisionMesh>(); break;
				case ColliderType::CAPSULE: /* when implemented */ break;
				}
			}

			ColliderType type = rBody.colliderType;

			if (type == ColliderType::NONE)
			{
				return;
			}

			ImGui::Combo("##LayerMask", (int*)&rBody.collider->layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));

			switch (type) {
			case ColliderType::BOX:
				if (auto* box = dynamic_cast<CollisionBox*>(rBody.collider.get())) {
					ImGui::InputFloat3("Extents", &box->extents.x);
				}
				break;
			case ColliderType::SPHERE:
				if (auto* sphere = dynamic_cast<CollisionSphere*>(rBody.collider.get())) {
					ImGui::InputFloat("Radius", &sphere->radius);
				}
				break;
			case ColliderType::TERRAIN:
				ImGui::TextDisabled("Terrain collider has no editable parameters.");
				break;
			case ColliderType::MESH:
				if (auto* mesh = dynamic_cast<CollisionMesh*>(rBody.collider.get())) {
					ImGui::Checkbox("Convex", &mesh->isConvex);
					// Optional: display mesh resource name, etc.
				}
				break;
			case ColliderType::CAPSULE:
				// Handle capsule here if you define its struct
				ImGui::Text("Capsule collider UI not implemented yet.");
				break;
			}
		});

		//displayComponent<CollisionBoxComponent>("Collision Box", [](CollisionBoxComponent& collisionBox) {
		//	ImGui::InputFloat("Half Extent", &collisionBox.halfExtent);
		//	ImGui::Combo("##LayerMask", (int*)&collisionBox.layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));
		//});

		//displayComponent<CollisionSphereComponent>("Collision Sphere", [](CollisionSphereComponent& collisionSphere) {
		//	ImGui::InputFloat("Radius", &collisionSphere.radius);
		//	ImGui::Combo("##LayerMask", (int*)&collisionSphere.layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));
		//	});

		//displayComponent<CollisionMeshComponent>("Collision Mesh", [](CollisionMeshComponent& collisionMesh) {
		//	ImGui::Combo("##LayerMask", (int*)&collisionMesh.layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));
		//	});

		displayComponent<PlayerController>("Player Controller", [](PlayerController& controller) {
			ImGui::TextDisabled("Controller has no editable parameters.");
			//ImGui::Combo("##LayerMask", (int*)&collisionMesh.layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));
			});

		displayComponent<MeshComponent>("Mesh", [](MeshComponent& meshComponent) {
			if (meshComponent.mesh.isEmpty()) return;

			BEGIN_IMGUI_TABLE("Mesh");

			addTableRow("Vertices count:", [&](std::string id) {
				rightAlignedText(std::to_string((int)meshComponent.mesh.get()->getNumOfVertices()).c_str());
			});

			addTableRowExt("Select Mesh", 
				[&](std::string id) 
				{
					if (ImGui::Button("Select Mesh"))
					{
						showMeshSelector = true;
					}
				},
				[&](std::string id) 
				{
					rightAlignedText(Engine::get()->getSubSystem<Assets>()->getAlias(meshComponent.mesh.getUID()).c_str());
				});

			END_IMGUI_TABLE();
			

			//ImGui::Text("Number of vertices: %d", (int)meshComponent.mesh.get()->getNumOfVertices());

			 //Button to trigger some action
			//if (ImGui::Button("Select Mesh")) 
			//{
			//	showMeshSelector = true;
			//}

			std::string selectedMeshUID;
			displaySelectMeshWindow(selectedMeshUID);

			if (!selectedMeshUID.empty())
			{
				meshComponent.mesh = Resource<MeshCollection>(selectedMeshUID);
			}

			//ImGui::SameLine();

			// Text display field
			//ImGui::Text(Engine::get()->getSubSystem<Assets>()->getAlias(meshComponent.mesh.getUID()).c_str());

			
			});

		displayComponent<CameraComponent>("Camera", [](CameraComponent& cameraComponent) {
			// TBD
			
			static const char* projectionMode[] = { "Perspective", "Orthographic" };
			static int currentProjection = 0; // Index of the selected item
			currentProjection = cameraComponent.type;

			ImGui::PushItemWidth(150.0f); // Set dropdown width to 150
			if (ImGui::BeginCombo("##ProjectionMode", projectionMode[currentProjection])) // Label for the combo box
			{
				for (int i = 0; i < IM_ARRAYSIZE(projectionMode); i++)
				{
					bool isSelected = (currentProjection == i);
					if (ImGui::Selectable(projectionMode[i], isSelected))
					{
						currentProjection = i; // Update selected index
						cameraComponent.type = (CameraComponent::CamType)currentProjection;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus(); // Set focus to the current item
				}
				ImGui::EndCombo();
			}

			ImGui::DragFloat("FOVY", &cameraComponent.fovy);
			ImGui::DragFloat("aspect", &cameraComponent.aspect);
			ImGui::DragFloat("z near", &cameraComponent.znear);
			ImGui::DragFloat("z far", &cameraComponent.zfar);
			});

		displayComponent<NativeScriptComponent>("Script", [](NativeScriptComponent& nsc) {
			// Button to trigger some action
			if (ImGui::Button("Select Script"))
			{
				showScriptSelector = true;
			}

			std::string selectedScript;
			displaySelectScriptWindow(selectedScript);

			if (!selectedScript.empty())
			{
				nsc.script = std::shared_ptr<ScriptableEntity>(NativeScriptsLoader::instance->getScript(selectedScript));
			}

			

			// Text display field
			if (nsc.script)
			{
				ImGui::SameLine();
				ImGui::Text(nsc.script->name().c_str());
			}
			});

		displayComponent<MaterialComponent>("Materials", [](MaterialComponent& materials) {
			int index = 0;
			for (auto& [id, mat] : materials)
			{
				ImGui::PushID(&mat);
				
				auto& matName = mat->getName();
				if (matName.empty())
				{
					matName = "Material " + std::to_string(index);
				}

				// Start a new collapsible header for each material
				if (ImGui::CollapsingHeader(matName.c_str()))
				{
					
					addSamplerEditWidget(mat, { 20,20 }, "Albedo", Texture::TextureType::Albedo);
					addSamplerEditWidget(mat, { 20,20 }, "Normal", Texture::TextureType::Normal);
					addSamplerEditWidget(mat, { 20,20 }, "Metallic", Texture::TextureType::Metallic);
					addSamplerEditWidget(mat, { 20,20 }, "Roughness", Texture::TextureType::Roughness);
					addSamplerEditWidget(mat, { 20,20 }, "Occlusion", Texture::TextureType::AmbientOcclusion);
				}
				++index;
				ImGui::PopID();
			}
			});

		displayComponent<DirectionalLight>("Directional Light", [](DirectionalLight& dLight) {
			auto& color = dLight.getColor();
			if (ImGui::ColorEdit3("Color", glm::value_ptr(color))) {
				dLight.SetColor(color);
			}
			});

		displayComponent<PointLight>("Point Light", [](PointLight& pLight) {
			auto& color = pLight.getColor();
			if (ImGui::ColorEdit3("Color", glm::value_ptr(color))) {
				pLight.SetColor(color);
			}

			Attenuation& attenuation = pLight.getAttenuation();
			ImGui::LabelText("", "Attenuation");
			ImGui::DragFloat("constant", &attenuation.constant, 0.01f);
			ImGui::DragFloat("linear", &attenuation.linear, 0.01f);
			ImGui::DragFloat("quadratic", &attenuation.quadratic, 0.01f);
			pLight.SetAttenuation(attenuation);
		});

		displayComponent<InstanceBatch>("Instance Batch", [](InstanceBatch& instanceBatch) {
			auto& transformations = instanceBatch.getTransformations();

			if (ImGui::Button("Add Transformation")) {
				instanceBatch.addTransformation(std::make_shared<Transformation>(state.getSelectedEntity()));
			}

			if (ImGui::BeginChild("Transformations List", ImVec2(0, 200), true)) {
				bool isChanged = false;
				for (size_t i = 0; i < transformations.size(); ++i) {
					if (ImGui::TreeNode((void*)(intptr_t)i, "Transformation %zu", i)) {
						displayTransformation(*transformations[i], isChanged);
						
						ImGui::TreePop();
					}
				}
				if (isChanged)
				{
					instanceBatch.build();
				}
				ImGui::EndChild();
			}
			});

		displayComponent<SkyboxComponent>("Skybox", [](SkyboxComponent& skybox) {
			addTextureEditWidget(skybox.originalImage, { 50, 50 }, [&](std::string uuid) {
				skybox.setSkybox(Resource<Texture>(uuid));
			});

			// Compile Button
			if (ImGui::Button("Build"))
			{
				skybox.build();
			}
		});

		displayComponent<ImageComponent>("Image", [](ImageComponent& image) {
			addTextureEditWidget(image.image, { 50, 50 }, [&](std::string uuid) {
				image.image = Resource<Texture>(uuid);
				});
			ImGui::DragFloat("posX", &image.position.x);
			ImGui::DragFloat("posY", &image.position.y);
			ImGui::DragFloat("sizeX", &image.size.x);
			ImGui::DragFloat("sizeY", &image.size.y);
			});

		displayComponent<Animator>("Animator", [](Animator& animator) {


			static int animIndex = 0;
			static std::string animName;
			static std::string selectedAnimUID;
			//static bool showAnimationSelector = false;
			//static std::vector<std::string> renameBuffer{};

			EntityState& eState = state.getCurrentEntityState();
			

			auto animations = animator.getAllAnimations();

			auto iter = animations.cbegin();

			// Display animation list
			int index = 0;
			while (iter != animations.cend())
			{
				auto& name = iter->first;
				auto& animation = iter->second;

				ImGui::PushID(index);

				std::string oldName = name;

				ImGui::InputText("##Name", &eState.animationRenameBuffers[index]);

				if (ImGui::IsItemDeactivatedAfterEdit()) {
					auto newAnimationName = std::string(eState.animationRenameBuffers[index]);
					// This runs when the user is done editing,
					// either by pressing Enter or unfocusing the input field
					if (newAnimationName != oldName) {
						auto anim = animator.getAnimation(oldName);
						animator.removeAnimation(oldName);
						animator.addAnimation(newAnimationName, anim);
						break;
					}
				}

				ImGui::SameLine();

				// Select animation button
				if (ImGui::Button("Select")) {
					animIndex = index;
					animName = name;
					showAnimationSelector = true;
				}

				ImGui::SameLine();
				ImGui::TextUnformatted(animation.getUID().c_str());

				ImGui::PopID();

				iter++;
				index++;
			}

			// Show animation selector popup (externally defined)
			if (showAnimationSelector) {
				displaySelectAnimationWindow(selectedAnimUID);
				if (!selectedAnimUID.empty()) {
					animator.addAnimation(animName, Resource<Animation>(selectedAnimUID));
					selectedAnimUID.clear();
					showAnimationSelector = false;
				}
			}

			if (ImGui::Button("Add Animation")) {
				animator.addAnimation("New Animation", Resource<Animation>::empty);
				eState.animationRenameBuffers.push_back("New Animation");
			}
			

			ImGui::DragFloat("playback speed", &animator.m_playbackSpeed);
			});

		displayComponent<Terrain>("Terrain", [](Terrain& terrain) {
			addTextureEditWidget(terrain.m_heightmap, { 50, 50 }, [&](std::string uuid) {
				terrain = Terrain::generateTerrain(terrain.m_width, terrain.m_height, terrain.m_scale, Resource<Texture>(uuid));
			});
			ImGui::DragInt("height", &terrain.m_height);
			ImGui::DragInt("width", &terrain.m_width);
			ImGui::DragInt("scale", &terrain.m_scale);


			ImGui::Separator();
			ImGui::LabelText("Textures", "");

			ImGui::SliderInt("Textures", &terrain.m_textureCount, 0, MAX_TEXTURE_COUNT);
			for (int i = 0; i < terrain.m_textureCount; i++)
			{
				ImGui::PushID(i);
				auto texture = terrain.getTexture(i);
				addTextureEditWidget(texture, { 50, 50 }, [i, &terrain](std::string uuid) {
					terrain.setTexture(i, Resource<Texture>(uuid));
				});
				ImGui::DragFloat("Height blend", &terrain.m_textureBlends[i].blend, .01f);
				auto scale = terrain.getTextureScale(i);
				if (ImGui::DragFloat("scaleX", &scale.r, .01f))
				{
					terrain.setTextureScaleX(i, scale.r);
				}
				if (ImGui::DragFloat("scaleY", &scale.g, .01f))
				{
					terrain.setTextureScaleY(i, scale.g);
				}
				ImGui::PopID();
			}
		});

		displayComponent<ShaderComponent>("Shader Component", [](ShaderComponent& shaderComponent) {
			//auto& state = g_states[selectedEntity.handlerID()];

			if (ImGui::Button("Select Shader"))
			{
				showShaderSelector = true;
			}

			std::string selectedShaderUID;
			displaySelectShaderWindow(selectedShaderUID);

			if (!selectedShaderUID.empty())
			{
				shaderComponent.setShader(Resource<Shader>(selectedShaderUID));
			}

			if (!shaderComponent.m_customShader.isEmpty())
			{
				auto name = Engine::get()->getSubSystem<Assets>()->getAlias(shaderComponent.m_customShader.getUID());
				ImGui::Text(name.c_str());
			}
			
			// Compile Button
			if (ImGui::Button("recompile"))
			{
				shaderComponent.m_customShader->recompile();
				shaderComponent.update();
			}

			ImGui::Separator();

			// Projection Type Drop-down
			const char* projectionTypes[] = { "Default", "Texture2D" };
			ImGui::Combo("Projection Type", (int*)&shaderComponent.projection, projectionTypes, IM_ARRAYSIZE(projectionTypes));

			if (shaderComponent.projection == ShaderComponent::ProjectionType::Texture2D)
			{
				// Projection Texture
				ImGui::Text("Projection Texture:");
				addTextureEditWidget(shaderComponent.projectionTexture, { 100,100}, [&](std::string uuid) {
					shaderComponent.setProjectionTexture(Resource<Texture>(uuid));
				});
			}

			// Custom Textures Array
			if (ImGui::CollapsingHeader("Textures"))
			{
				for (auto& [name, texture] : shaderComponent.customTextures)
				{
					ImGui::PushID(name.c_str());
					ImGui::Text(name.c_str());
					addTextureEditWidget(texture, { 100,100 }, [&](std::string uuid) {
						texture = Resource<Texture>(uuid);
						});
					ImGui::PopID();
				}
				
			}

			// Display Uniforms and Update Shader
			if (ImGui::CollapsingHeader("Uniforms"))
			{
				for (auto& [name, value] : shaderComponent.m_uniformProperties)
				{
					ImGui::PushID(name.c_str());
					bool updated = false; // Track if the value was changed
					std::visit([&](auto& v)
						{
							using T = std::decay_t<decltype(v)>;
							ImGui::Text("%s:", name.c_str());

							if constexpr (std::is_same_v<T, float>)
							{
								updated = ImGui::DragFloat(("##" + name).c_str(), &v, 0.1f);
							}
							else if constexpr (std::is_same_v<T, glm::vec2>)
							{
								updated = ImGui::DragFloat2(("##" + name).c_str(), &v[0], 0.1f);
							}
							else if constexpr (std::is_same_v<T, glm::vec3>)
							{
								updated = ImGui::DragFloat3(("##" + name).c_str(), &v[0], 0.1f);
							}
							else if constexpr (std::is_same_v<T, glm::vec4>)
							{
								updated = ImGui::DragFloat4(("##" + name).c_str(), &v[0], 0.1f);
							}
							else if constexpr (std::is_same_v<T, int>)
							{
								updated = ImGui::InputInt(("##" + name).c_str(), &v);
							}
							else if constexpr (std::is_same_v<T, unsigned int>)
							{
								updated = ImGui::InputScalar(("##" + name).c_str(), ImGuiDataType_U32, &v);
							}
							else if constexpr (std::is_same_v<T, glm::mat3>)
							{
								for (int i = 0; i < 3; ++i)
									updated |= ImGui::DragFloat3((name + "##row" + std::to_string(i)).c_str(), &v[i][0], 0.1f);
							}
							else if constexpr (std::is_same_v<T, glm::mat4>)
							{
								for (int i = 0; i < 4; ++i)
									updated |= ImGui::DragFloat4((name + "##row" + std::to_string(i)).c_str(), &v[i][0], 0.1f);
							}
						}, value);

					// If the value changed, update the shader
					if (updated)
					{
						shaderComponent.m_customShader->setUniformValue(name, value);
					}

					ImGui::PopID();
				}
			}
		});

		displayComponent<TestComp>("Test Component", [](TestComp& testComp) {
			});

		if (ImGui::Button("Add Component", ImVec2(windowWidth, 0)))
		{
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (ImGui::BeginPopup("AddComponentPopup"))
		{
			if (ImGui::MenuItem("Transformation"))
			{
				state.getSelectedEntity().addComponent<Transformation>(state.getSelectedEntity());
			}

			if (ImGui::MenuItem("Physics"))
			{
				state.getSelectedEntity().addComponent<PhysicsComponent>();
			}

			if (ImGui::MenuItem("Player Controller"))
			{
				state.getSelectedEntity().addComponent<PlayerController>();
			}

			//if (ImGui::MenuItem("Collision Box"))
			//{
			//	selectedEntity.addComponent<CollisionBoxComponent>();
			//}

			//if (ImGui::MenuItem("Collision Sphere"))
			//{
			//	selectedEntity.addComponent<CollisionSphereComponent>();
			//}

			//if (ImGui::MenuItem("Collision Mesh"))
			//{
			//	throw std::runtime_error("Not implemented");
			//	// TODO fix
			//	//auto meshComponent = selectedEntity.tryGetComponent<MeshComponent>();
			//	//if (meshComponent)
			//	//{
			//	//	auto& meshCollisions = selectedEntity.addComponent<CollisionMeshComponent>();
			//	//	meshCollisions.mesh = meshComponent->mesh;
			//	//}
			//}

			//if (ImGui::MenuItem("Collision Terrain"))
			//{
			//	selectedEntity.addComponent<CollisionTerrainComponent>();
			//}

			if (ImGui::MenuItem("Mesh"))
			{
				state.getSelectedEntity().addComponent<MeshComponent>();
			}

			if (ImGui::MenuItem("Camera"))
			{
				state.getSelectedEntity().addComponent<CameraComponent>();
			}

			if (ImGui::MenuItem("Script"))
			{
				state.getSelectedEntity().addComponent<NativeScriptComponent>();
			}

			if (ImGui::MenuItem("InstanceBatch"))
			{
				auto meshComponent = state.getSelectedEntity().tryGetComponent<MeshComponent>();
				if (meshComponent)
				{
					throw std::runtime_error("Not implemented");
					// TODO fix
					//selectedEntity.addComponent<InstanceBatch>(std::vector<std::shared_ptr<Transformation>>{}, meshComponent->mesh);
				}
			}

			if (ImGui::MenuItem("Image"))
			{
				auto& img = state.getSelectedEntity().addComponent<ImageComponent>(Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::WHITE_1X1));
				img.size = { 50, 50 };
			}

			if (ImGui::MenuItem("Animator"))
			{
				auto& animator = state.getSelectedEntity().addComponent<Animator>();
			}

			if (ImGui::MenuItem("Terrain"))
			{
				auto& terrain = state.getSelectedEntity().addComponent<Terrain>();
			}

			if (ImGui::MenuItem("Skybox"))
			{
				auto& skybox = state.getSelectedEntity().addComponent<SkyboxComponent>();
			}

			if (ImGui::MenuItem("Shader"))
			{
				auto& shader = state.getSelectedEntity().addComponent<ShaderComponent>();
			}

			//Todo REMOVE
			if (ImGui::MenuItem("TestComponent"))
			{
				auto& testComp = state.getSelectedEntity().addComponent<TestComp>();
			}

			ImGui::EndPopup();
		}
		
	}

	// Render inspector content here
	ImGui::End();
}

void RenderAssetViewWindow(float width, float height) {
	

	auto assets = Engine::get()->getSubSystem<Assets>();
	float windowWidth = width - 10;
	float startX = 5; // Add a gap of 5 pixels
	float startY = height * 0.7f + 30; // Adjust vertical position to place it below the "Scene Hierarchy" window
	ImGui::SetNextWindowPos(ImVec2(startX, startY));
	ImGui::SetNextWindowSize(ImVec2(windowWidth, height * 0.3f - 35)); // Adjust height as needed
	ImGui::Begin("Asset View", nullptr, style);
	ImVec2 listBoxSize(windowWidth, height * 0.3f - 35);



	displayWindowHeader("Assets");


	static std::filesystem::path cwd = Engine::get()->getProjectDirectory();

	bool canGoBack = cwd != Engine::get()->getProjectDirectory();

	if (!canGoBack)
	{
		// Make button look disabled
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f); // 50% transparency
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray color
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
	}

	// Back button
	bool clicked = ImGui::Button("<-", ImVec2(30, 30));

	// Restore style if it was pushed
	if (!canGoBack)
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
	}

	// Only handle click if it's allowed
	if (clicked && canGoBack)
	{
		cwd = cwd.parent_path();
	}


	ImGui::SameLine();

	// Header (fixed at top)
	ImGui::SetWindowFontScale(1.3f);
	ImGui::Text("%s", cwd.string().c_str());
	ImGui::SetWindowFontScale(1.0f);
	ImGui::Separator();

	// Scrollable region
	ImGui::BeginChild("FileBrowserScrollingRegion", ImVec2(0, 0), false);

	// Start grid layout
	const float thumbnailSize = 64.0f;
	const float padding = 16.0f;
	const int columns = 6; // number of thumbnails per row

	ImGui::Columns(columns, nullptr, false); // begin columns

	for (const auto& entry : std::filesystem::directory_iterator(cwd))
	{
		const std::string filenameFull = entry.path().filename().string();

		std::string filename = entry.path().stem().string();

		if (entry.is_directory())
		{
			ImGui::BeginGroup(); // Begin entry group (icon + name + extra info)

			std::string dirName = "[Dir] " + filename;
			// Create a small icon

			unsigned int iconID = icons.at("folder")->getID();
			ImGui::Image((ImTextureID)iconID, ImVec2(32, 32));
			ImGui::SameLine();

			// Draw filename and small info
			ImGui::Text("%s", dirName.c_str());

			ImGui::EndGroup();

			ImGui::NextColumn(); // move to next grid slot

			//ImGui::Separator(); // nice line between items
		}

		

		// Double click to open
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			cwd /= entry.path().filename();
			break;
		}

		

	}

	for (const auto& entry : std::filesystem::directory_iterator(cwd))
	{
		const std::string filenameFull = entry.path().filename().string();

		if (filenameFull == "entities.json" || filenameFull == "ProjectAssetRegistry.json")
			continue; // Skip unwanted files

		std::string filename = entry.path().stem().string();
		
		if (entry.is_regular_file())
		{
			if (!assets->hasAsset(filename)) continue;

			ImGui::BeginGroup(); // Begin entry group (icon + name + extra info)

			const AssetInfo& aInfo = assets->getAsset(filename);

			int iconID = 0;
			if (aInfo.aType == AssetType::MESH)
			{
				iconID = icons.at("mesh")->getID();
			}
			else if (aInfo.aType == AssetType::TEXTURE)
			{
				iconID = icons.at("texture")->getID();
			}
			else if (aInfo.aType == AssetType::ANIMATION)
			{
				iconID = icons.at("animation")->getID();
			}
			else if (aInfo.aType == AssetType::SHADER)
			{
				iconID = icons.at("shader")->getID();
			}

			// Create a small icon
			ImGui::Image((ImTextureID)iconID, ImVec2(32, 32));
			ImGui::SameLine();

			std::string assetName = "[" + getAssetTypeAsStr(aInfo.aType) + "] " + aInfo.name;

			// Draw filename and small info
			ImGui::Text("%s", assetName.c_str());

			auto fileSize = std::filesystem::file_size(entry);
			ImGui::SameLine();
			ImGui::TextDisabled("(%.1f KB)", fileSize / 1024.0f);

			ImGui::EndGroup();

			//ImGui::Separator(); // nice line between items

			ImGui::NextColumn(); // move to next grid slot
		}



		// Double click to open
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{

		}
		
		
	}
	ImGui::Columns(1); // end columns

	ImGui::EndChild(); // end scrollable region
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
	for (const auto& [name, textureResource] : debugTextures)
	{
		// Show image button
		ImGui::BeginGroup();
		ImGui::PushID(name.c_str());

		if (ImGui::CollapsingHeader(name.c_str()))
		{
			ImTextureID texID = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(textureResource.get()->getID()));
			if (ImGui::ImageButton(texID, ImVec2(100, 100)))
			{
				selectedTextureName = name;
				showTextureDisplayWindow = true;
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

						// Command to execute the Python script with folderPath as an argument
						std::string command = "python \"" + pythonScriptPath + "\" \"" + Engine::get()->getInitParams().projectDir + "\"";

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
							showTextureCreateWindow = true;
							
						}
						if (ImGui::MenuItem("Shader Override")) {
							showShaderCreateWindow = true;

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
		
		// Render UI
		RenderSimulationControlView(screenWidth, screenHeight);
		RenderViewWindow(screenWidth, screenHeight);
		RenderSceneHierarchyWindow(screenWidth, screenHeight);
		RenderInspectorWindow(screenWidth, screenHeight);
		RenderAssetViewWindow(screenWidth, screenHeight); // Add the Asset View window
		ShowTextureCreatorWindow();
		ShowShaderCreatorWindow();
		ShowTextureDisplayWindow();

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
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.43f, 0.70f, 1.00f);

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
		g_primaryCamera = scene->getActiveCamera();

		// set Editor camera as active camera
		auto editorCamera = m_editorRegistry->createEntity("Editor Camera");
		editorCamera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)4 / 3, 0.1f, 3000.0f));
		editorCamera.addComponent<NativeScriptComponent>().bind<EditorCamera>();
		auto& nsc = editorCamera.getComponent<NativeScriptComponent>();
		nsc.script->eventHandler = Engine::get()->getEventSystem()->bindToLayer(uiLayer->name);
		Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(editorCamera);

		nsc.script->onCreate();

		g_editorCamera = editorCamera;

		Engine::get()->getInput()->getKeyboard()->onKeyPressed(gameHandler, SDL_SCANCODE_ESCAPE, [](SDL_Event e) { stopSimulation(); });
		
		Engine::get()->getInput()->getKeyboard()->onKeyReleased(uiHandler, SDL_SCANCODE_X, [](SDL_Event e) {
			debugTerrainFlag = true;
			});

		//g_previewWindowID = Engine::get()->getContext()->getActiveScene()->addRenderView(0, 0, 300, 200, g_primaryCamera);
		g_previewWindowID = Engine::get()->getContext()->getActiveScene()->addRenderView("Game Preview", 0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight(), g_primaryCamera);

		updateScene();

		auto gui = new GUI_Helper();
		Engine::get()->getImguiHandler()->addGUI(gui);

		

		

		Texture::TextureImportSettings settings;
		settings.isTransient = true;
		icons["mesh"] = Texture::importTexture2D(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-cube-100.png", settings);
		icons["texture"] = Texture::importTexture2D(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-image-100.png", settings);
		icons["animation"] = Texture::importTexture2D(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-skeleton-100.png", settings);
		icons["shader"] = Texture::importTexture2D(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-code-100.png", settings);
		icons["folder"] = Texture::importTexture2D(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-folder-100.png", settings);
	}

	void update(float deltaTime) override
	{
		g_editorCamera.getComponent<NativeScriptComponent>().script->onUpdate(deltaTime);
	}
	std::shared_ptr<SGE_Regsitry> m_editorRegistry;
	
	EventHandler uiHandler;
	EventHandler gameHandler;
};

Application* CreateApplication()
{
	return new EditorApp();
}