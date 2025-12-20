#include "TransformTool.h"

#include "sge.h"

#include "imgui.h"

#include "EditorState.h"
#include "EntityState.h"

extern Entity g_editorCamera;

void TransformTool::update(ImVec2 windowPos, ImVec2 viewportSize)
{
	ImVec2 renderViewWindowSize = viewportSize;

	float innerWindowWidth = renderViewWindowSize.x;
	float innerWindowHeight = 35.0f;
	ImVec2 toolbarPos(windowPos.x + 10, windowPos.y + 30);

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

		static const char* renderModeOptions[] = { "Shaded", "Wireframe" };
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

		glm::vec3 camPos = primaryCameraTransform.getWorldPosition();
		auto camView = glm::lookAt(camPos, camPos + primaryCamera.front, primaryCamera.up);
		const float* camViewPtr = glm::value_ptr(camView);

		auto projection = primaryCamera.getProjection();
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
		transform.setLocalRotation(glm::vec3(Constants::toRadians * matrixRotation[0], Constants::toRadians * matrixRotation[1], Constants::toRadians * matrixRotation[2]));
		transform.setLocalScale(glm::vec3(matrixScale[0], matrixScale[1], matrixScale[2]));
	}
}

const char* TransformTool::name() const
{
    return "TransformTool";
}
