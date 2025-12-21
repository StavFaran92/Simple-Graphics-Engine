#include "SceneViewWindow.h"

#include "EditorState.h"
#include "EntityState.h"

extern Entity g_editorCamera;

void SceneViewWindow::display()
{
	ImGui::Begin("View", nullptr, windowFlags);

	ImVec2 viewportMin = ImGui::GetWindowContentRegionMin();
	ImVec2 viewportMax = ImGui::GetWindowContentRegionMax();
	ImVec2 windowPos = ImGui::GetWindowPos();

	EditorState::Instance().sceneViewRect = {
		{windowPos.x + viewportMin.x, windowPos.y + viewportMin.y},
		{windowPos.x + viewportMax.x, windowPos.y + viewportMax.y}
	};

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

							EditorState::Instance().setActiveEditorTool(EditorTool::Type::TransformTool);

							break;
						}
					}
				}
			}
		}
	}

	auto activeEditorTool = EditorState::Instance().getActiveEditorTool();
	if (activeEditorTool)
	{
		activeEditorTool->update(windowPos, renderViewWindowSize);
	}

	if (!Engine::get()->getContext()->getActiveScene()->isSimulationActive())
	{
		if (state.getSelectedEntity() != Entity::EmptyEntity &&
			state.getSelectedEntity().HasComponent<CameraComponent>() &&
			state.getSelectedEntity() == Engine::get()->getContext()->getActiveScene()->getGameCamera())
		{
			// This is a shit hack and it will break at some point in the future, should use events or something as a better solution.
			Engine::get()->getContext()->getActiveScene()->setGameRenderViewEnabled(true);

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
		else
		{
			Engine::get()->getContext()->getActiveScene()->setGameRenderViewEnabled(false);
		}
	}


	ImGui::End();
}