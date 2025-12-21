#include "UIEntitySelectionLayer.h"

#include "EditorState.h"
#include "EntityState.h"

extern Entity g_editorCamera;


bool UIEntitySelectionLayer::handleEvent(SDL_Event e)
{
	if (!Engine::get()->getContext()->getActiveScene()->isSimulationActive())
	{
		if (e.type == SDL_MOUSEBUTTONUP && static_cast<MouseButton>(e.button.button) == MouseButton::MOUSE_BUTTON_LEFT)
		{

			if (EditorState::Instance().isMouseInSceneView)
			{
				ImVec2 mousePos = ImGui::GetMousePos();
				glm::vec2 viewportPos = EditorState::Instance().sceneViewRect.min;
				glm::vec2 renderViewWindowSize = EditorState::Instance().sceneViewRect.max;
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
	return false;
}