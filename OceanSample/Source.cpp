#include "EntryPoint.h"
#include "sge.h"

#include "GUIHandler.h"


class Sandbox : public Application
{
public:
	void start() override
	{
		ImGui::SetCurrentContext((ImGuiContext*)Engine::get()->getImguiHandler()->getCurrentContext());

		auto importer = getContext()->getModelImporter();
		auto quad = importer->loadModelFromFile("C:/Users/Stav/Documents/blender/plane_v2.fbx", getContext()->getActiveScene().get());

		auto& planeTransform = quad.getComponent<Transformation>();
		planeTransform.rotate({ 1,0,0 }, 90);
		planeTransform.rotate({ 0,1,0 }, 90);
		planeTransform.scale({ 10, 10, 1 });

		auto& shader = ShaderBuilder::create("Resources/Content/Shaders/OceanShader.glsl").build();

		quad.addComponent<ShaderComponent>(shader);

		shader.m_vertexShader->setUniformValue("amplitude", 0.5f);
		shader.m_vertexShader->setUniformValue("waveDirection", glm::vec2(1, 0));
		shader.m_vertexShader->setUniformValue("waveLength", 2.0f);
		shader.m_vertexShader->setUniformValue("waveSpeed", 5.0f);
		shader.m_vertexShader->setUniformValue("steepness", .5f);

		auto gui = new GUIHandler(shader.m_vertexShader);
		Engine::get()->getImguiHandler()->addGUI(gui);

	}

};

Application* CreateApplication()
{
	return new Sandbox();
}