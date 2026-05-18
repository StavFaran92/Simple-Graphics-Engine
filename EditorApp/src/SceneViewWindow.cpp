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