#include "EntryPoint.h"
#include "sge.h"

#include "GUIHandler.h"
#include "NativeScripts.h"

class OceanGUI : public GuiMenu
{
public:
	OceanGUI(Resource<Shader> shader)
	{
		auto ctx = ImGui::GetCurrentContext();

		ImGui::SetCurrentContext(ctx);

		m_shader = shader;
	}
	void display() override
	{
		ImGui::SetNextWindowPos({ 10, 10 }, ImGuiCond_Once);
		ImGui::SetNextWindowSize({ 300, 250 }, ImGuiCond_Once);
		ImGui::Begin("Gretsner wave Controller", 0, ImGuiWindowFlags_NoResize);

		ImGui::PushItemWidth(100);

		static float amplitude = 0.5f;

		if (ImGui::InputFloat("amplitude", &amplitude))
			m_shader->setUniformValue("amplitude", amplitude);

		static auto waveDir = glm::vec2(1, 0);

		if (ImGui::InputFloat2("wave Direction (x,y)", glm::value_ptr(waveDir)))
			m_shader->setUniformValue("waveDirection", waveDir);

		static float waveLength = 2.0f;

		if (ImGui::InputFloat("wave Length", &waveLength))
			m_shader->setUniformValue("waveLength", waveLength);

		static float waveSpeed = 5.0f;

		if (ImGui::InputFloat("wave Speed", &waveSpeed))
			m_shader->setUniformValue("waveSpeed", waveSpeed);

		static float steepness = .5f;

		if (ImGui::SliderFloat("Steepness", &steepness, 0, 1))
			m_shader->setUniformValue("steepness", steepness);

		ImGui::PopItemWidth();


		ImGui::End();
	}

private:
	Resource<Shader> m_shader = nullptr;
};

class PoolGUI : public GuiMenu
{
public:
	PoolGUI(Resource<Shader> shader)
	{
		auto ctx = ImGui::GetCurrentContext();

		ImGui::SetCurrentContext(ctx);

		m_shader = shader;
	}
	void display() override
	{
		ImGui::SetNextWindowPos({ 10, 10 }, ImGuiCond_Once);
		ImGui::SetNextWindowSize({ 300, 250 }, ImGuiCond_Once);
		ImGui::Begin("Pool Controller", 0, ImGuiWindowFlags_NoResize);

		ImGui::PushItemWidth(100);

		if (ImGui::Button("Recompile shader"))
		{
			m_shader->recompile();
		}

		if (ImGui::Button("reload config"))
		{
			Engine::get()->reloadEngineConfig();
		}

		ImGui::PopItemWidth();

		

		ImGui::End();
	}

private:
	Resource<Shader> m_shader = nullptr;
};


Entity camera;

class Sandbox : public Application
{
public:
	void start() override
	{
		Engine::get()->getContext()->getActiveScene()->startSimulation();

		ImGui::SetCurrentContext((ImGuiContext*)Engine::get()->getImguiHandler()->getCurrentContext());

		//camera = Engine::get()->getContext()->getActiveScene()->createEntity("Editor Camera");
		//camera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)4 / 3, 0.1f, 3000.0f));
		//camera.addComponent<NativeScriptComponent>().bind<CameraScript>();
		//camera.getComponent<Transformation>().setLocalPosition(glm::vec3(3, 3, 3));
		////camera.getComponent<Transformation>().setLocalRotation().set(glm::vec3(3, 3, 3));
		//Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(camera);

		auto skybox = Engine::get()->getContext()->getActiveScene()->createEntity("Skybox");
		
		Texture::TextureImportSettings importSettings;
		importSettings.flip = true;
		auto skyboxTexture = Texture::importTexture2D("C:/Users/Stav/Downloads/sunflowers_puresky_4k.hdr", importSettings);
		auto& skyboxComponent = skybox.addComponent<SkyboxComponent>();
		skyboxComponent.setSkybox(skyboxTexture);
		skyboxComponent.build();



		auto editorCamera = Engine::get()->getContext()->getActiveScene()->createEntity("Camera");
		editorCamera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)4 / 3, 0.1f, 3000.0f));
		editorCamera.addComponent<NativeScriptComponent>().bind<CameraScript>();
		auto& nsc = editorCamera.getComponent<NativeScriptComponent>();
		Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(editorCamera);
		nsc.script->onCreate();



		auto quad = Engine::get()->getContext()->getActiveScene()->createEntity();
		auto quadModel = Engine::get()->getSubSystem<ModelImporter>()->import("Resources/Content/Meshes/sd_plane.fbx");

		quad.addComponent<MeshComponent>(quadModel.mesh);
		quad.addComponent<RenderableComponent>();

		auto& planeTransform = quad.getComponent<Transformation>();
		planeTransform.rotate({ 1,0,0 }, 90);
		planeTransform.rotate({ 0,1,0 }, 90);
		planeTransform.rotate({ 1,0,0 }, 180);
		planeTransform.scale({ 100, 100, 1 });

		createPool(quad);

		//camera.getComponent<NativeScriptComponent>().script->onCreate();
	}

	void update(float deltaTime) override
	{
		//camera.getComponent<NativeScriptComponent>().script->onUpdate(deltaTime);
	}

private:
	void createPool(Entity quad)
	{
		auto& shader = Shader::createOverrideShader("WaterShader", "../../OceanSample/Resources/Content/Shaders/WaterShader.glsl", ShaderOverride::PBR);

		

		auto& shaderComponent = quad.addComponent<ShaderComponent>();
		shaderComponent.setShader(shader);

		auto waterNormal = Texture::importTexture2D("../../OceanSample/Resources/Content/Textures/water_new_height.png");
		//shader->setTextureInShader(waterNormal, "waterNormalSampler", 0);
		shaderComponent.addTexture("waterNormalSampler", waterNormal);

		//quad.getComponent<MaterialComponent>().materials[0]->setTexture(Texture::Type::Roughness, Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::BLACK_1X1));

		auto gui = new PoolGUI(shader);
		Engine::get()->getImguiHandler()->addGUI(gui);
	}

	void createOcean(Entity quad)
	{
		auto& shader = Shader::createOverrideShader("OceanShader", "../../OceanSample/Resources/Content/Shaders/OceanShader.glsl", ShaderOverride::PBR);
		//auto& shader = CustomShaderBuilder::create("Resources/Content/Shaders/OceanShader.glsl", ShaderOverride::PBR).build();

		auto& shaderComponent = quad.addComponent<ShaderComponent>();
		shaderComponent.setShader(shader);

		shader->setUniformValue("amplitude", 3.5f);
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