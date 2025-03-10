#include "EntryPoint.h"
#include "sge.h"

#include "GUIHandler.h"



class Sandbox : public Application
{
public:
	void start() override
	{
		ImGui::SetCurrentContext((ImGuiContext*)Engine::get()->getImguiHandler()->getCurrentContext());

		auto quad = Engine::get()->getContext()->getActiveScene()->createEntity("quad");
		auto quadModel = Engine::get()->getSubSystem<ModelImporter>()->import("C:/Users/Stav/Documents/blender/plane_v2.fbx");

		quad.addComponent<MeshComponent>(quadModel.mesh);
		auto& planeTransform = quad.getComponent<Transformation>();
		planeTransform.rotate({ 1,0,0 }, 90);
		planeTransform.rotate({ 0,1,0 }, 90);
		//planeTransform.rotate({ 1,0,0 }, 180);
		planeTransform.scale({ 100, 100, 1 });

		createPool(quad);
	}

private:
	void createPool(Entity quad)
	{
		auto& shader = ShaderBuilder::create("Resources/Content/Shaders/WaterShader.glsl").build();

		auto brickTexture = Engine::get()->getSubSystem<Assets>()->importTexture2D("Resources/Content/Textures/water_new_height.png");
		shader.addTexture("waterNormalSampler", brickTexture);

		quad.addComponent<ShaderComponent>(shader);

		//quad.getComponent<MaterialComponent>().materials[0]->setTexture(Texture::TextureType::Roughness, Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::BLACK_1X1));

		auto gui = new GUIHandler(shader.m_vertexShader);
		Engine::get()->getImguiHandler()->addGUI(gui);
	}

	void createOcean(Entity quad)
	{
		auto& shader = ShaderBuilder::create("Resources/Content/Shaders/OceanShader.glsl").build();

		quad.addComponent<ShaderComponent>(shader);

		shader.m_vertexShader->setUniformValue("amplitude", 0.5f);
		shader.m_vertexShader->setUniformValue("waveDirection", glm::vec2(1, 0));
		shader.m_vertexShader->setUniformValue("waveLength", 2.0f);
		shader.m_vertexShader->setUniformValue("waveSpeed", 1.0f);
		shader.m_vertexShader->setUniformValue("steepness", .5f);

		//quad.getComponent<MaterialComponent>().materials[0]->setTexture(Texture::TextureType::Roughness, Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::BLACK_1X1));
	}

};

Application* CreateApplication()
{
	return new Sandbox();
}