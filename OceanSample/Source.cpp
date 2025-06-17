#include "EntryPoint.h"
#include "sge.h"

#include "GUIHandler.h"
#include "NativeScripts.h"

Entity camera;

class Sandbox : public Application
{
public:
	void start() override
	{
		ImGui::SetCurrentContext((ImGuiContext*)Engine::get()->getImguiHandler()->getCurrentContext());

		camera = Engine::get()->getContext()->getActiveScene()->createEntity("Editor Camera");
		camera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)4 / 3, 0.1f, 3000.0f));
		camera.addComponent<NativeScriptComponent>().bind<CameraScript>();
		camera.getComponent<Transformation>().setLocalPosition(glm::vec3(3, 3, 3));
		//camera.getComponent<Transformation>().setLocalRotation().set(glm::vec3(3, 3, 3));
		Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(camera);

		auto quad = Engine::get()->getContext()->getActiveScene()->createEntity();
		auto quadModel = Engine::get()->getSubSystem<ModelImporter>()->import("Resources/Content/Meshes/sd_plane.fbx");

		quad.addComponent<MeshComponent>(quadModel.mesh);
		quad.addComponent<RenderableComponent>();

		auto& planeTransform = quad.getComponent<Transformation>();
		planeTransform.rotate({ 1,0,0 }, 90);
		planeTransform.rotate({ 0,1,0 }, 90);
		//planeTransform.rotate({ 1,0,0 }, 180);
		planeTransform.scale({ 100, 100, 1 });

		createOcean(quad);

		//camera.getComponent<NativeScriptComponent>().script->onCreate();
	}

	void update(float deltaTime) override
	{
		//camera.getComponent<NativeScriptComponent>().script->onUpdate(deltaTime);
	}

private:
	void createPool(Entity quad)
	{
		auto& shader = CustomShaderBuilder::create("Resources/Content/Shaders/WaterShader.glsl", ShaderOverride::PBR).build();

		auto brickTexture = Texture::importTexture2D("Resources/Content/Textures/water_new_height.png");
		shader.addTexture("waterNormalSampler", brickTexture);

		quad.addComponent<ShaderComponent>(shader);

		//quad.getComponent<MaterialComponent>().materials[0]->setTexture(Texture::Type::Roughness, Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::BLACK_1X1));

		auto gui = new GUIHandler(shader.m_customShader.get());
		Engine::get()->getImguiHandler()->addGUI(gui);
	}

	void createOcean(Entity quad)
	{
		auto& shader = Shader::createOverrideShader("OceanShader", "Resources/Content/Shaders/OceanShader.glsl", ShaderOverride::PBR);
		//auto& shader = CustomShaderBuilder::create("Resources/Content/Shaders/OceanShader.glsl", ShaderOverride::PBR).build();

		auto& shaderComponent = quad.addComponent<ShaderComponent>();
		shaderComponent.setShader(shader);

		shader->setUniformValue("amplitude", 0.5f);
		shader->setUniformValue("waveDirection", glm::vec2(1, 0));
		shader->setUniformValue("waveLength", 2.0f);
		shader->setUniformValue("waveSpeed", 1.0f);
		shader->setUniformValue("steepness", .5f);

		//std::shared_ptr<Material> mat = std::make_shared<Material>();
		//quad.addComponent<MaterialComponent>().addMaterial().materials[0]->setTexture(Texture::Type::Roughness, Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::BLACK_1X1));
	}

};

Application* CreateApplication()
{
	return new Sandbox();
}