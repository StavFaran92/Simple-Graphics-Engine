#include "EntryPoint.h"
#include "sge.h"

//#include "GUIHandler.h"
#include "NativeScripts.h"

const std::string FOLIAGE_ROOT_DIR = "../../FoliageSample/Resources/";

//class OceanGUI : public GuiMenu
//{
//public:
//	OceanGUI(Resource<Shader> shader)
//	{
//		auto ctx = ImGui::GetCurrentContext();
//
//		ImGui::SetCurrentContext(ctx);
//
//		m_shader = shader;
//	}
//	void display() override
//	{
//		ImGui::SetNextWindowPos({ 10, 10 }, ImGuiCond_Once);
//		ImGui::SetNextWindowSize({ 300, 250 }, ImGuiCond_Once);
//		ImGui::Begin("Gretsner wave Controller", 0, ImGuiWindowFlags_NoResize);
//
//		ImGui::PushItemWidth(100);
//
//		static float amplitude = 0.5f;
//
//		if (ImGui::InputFloat("amplitude", &amplitude))
//			m_shader->setUniformValue("amplitude", amplitude);
//
//		static auto waveDir = glm::vec2(1, 0);
//
//		if (ImGui::InputFloat2("wave Direction (x,y)", glm::value_ptr(waveDir)))
//			m_shader->setUniformValue("waveDirection", waveDir);
//
//		static float waveLength = 2.0f;
//
//		if (ImGui::InputFloat("wave Length", &waveLength))
//			m_shader->setUniformValue("waveLength", waveLength);
//
//		static float waveSpeed = 5.0f;
//
//		if (ImGui::InputFloat("wave Speed", &waveSpeed))
//			m_shader->setUniformValue("waveSpeed", waveSpeed);
//
//		static float steepness = .5f;
//
//		if (ImGui::SliderFloat("Steepness", &steepness, 0, 1))
//			m_shader->setUniformValue("steepness", steepness);
//
//		ImGui::PopItemWidth();
//
//
//		ImGui::End();
//	}
//
//private:
//	Resource<Shader> m_shader = nullptr;
//};
//
//class PoolGUI : public GuiMenu
//{
//public:
//	PoolGUI(Resource<Shader> shader)
//	{
//		auto ctx = ImGui::GetCurrentContext();
//
//		ImGui::SetCurrentContext(ctx);
//
//		m_shader = shader;
//	}
//	void display() override
//	{
//		ImGui::SetNextWindowPos({ 10, 10 }, ImGuiCond_Once);
//		ImGui::SetNextWindowSize({ 300, 250 }, ImGuiCond_Once);
//		ImGui::Begin("Pool Controller", 0, ImGuiWindowFlags_NoResize);
//
//		ImGui::PushItemWidth(100);
//
//		if (ImGui::Button("Recompile shader"))
//		{
//			m_shader->recompile();
//		}
//
//		if (ImGui::Button("reload config"))
//		{
//			Engine::get()->reloadEngineConfig();
//		}
//
//		ImGui::PopItemWidth();
//
//		
//
//		ImGui::End();
//	}
//
//private:
//	Resource<Shader> m_shader = nullptr;
//};


class Sandbox : public Application
{
public:
	void start() override
	{
		auto handler = Engine::get()->getEventSystem()->bindToLayer("GameLayer");
		Engine::get()->getInput()->getKeyboard()->onKeyPressed(handler, SDL_SCANCODE_ESCAPE, [](SDL_Event e) { Engine::get()->stop(); });

		Engine::get()->getContext()->getActiveScene()->startSimulation();

		ImGui::SetCurrentContext((ImGuiContext*)Engine::get()->getImguiHandler()->getCurrentContext());

		//auto skybox = Engine::get()->getContext()->getActiveScene()->createEntity("Skybox");
		//
		//Texture::TextureImportSettings importSettings;
		//importSettings.flip = true;
		//auto skyboxTexture = Texture::importTexture2D("C:/Users/Stav/Downloads/sunflowers_puresky_4k.hdr", importSettings);
		//auto& skyboxComponent = skybox.addComponent<SkyboxComponent>();
		//skyboxComponent.setSkybox(skyboxTexture);
		//skyboxComponent.build();



		auto editorCamera = Engine::get()->getContext()->getActiveScene()->createEntity("Camera");
		editorCamera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)4 / 3, 0.1f, 3000.0f));
		editorCamera.addComponent<NativeScriptComponent>().bind<CameraScript>();
		auto& nsc = editorCamera.getComponent<NativeScriptComponent>();
		Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(editorCamera);
		nsc.script->onCreate();

		Entity grass = Engine::get()->getContext()->getActiveScene()->createEntity("Grass");
		auto& foliage = grass.addComponent<FoliageComponent>();
		foliage.density = 1 / 255.f;
		foliage.m_foliageSpreadMap = Texture::importTexture2D(FOLIAGE_ROOT_DIR + "Content/Textures/sample_grass_spread.png");
		foliage.build();

		

		//camera.getComponent<NativeScriptComponent>().script->onCreate();
	}

	void update(float deltaTime) override
	{
		//camera.getComponent<NativeScriptComponent>().script->onUpdate(deltaTime);
	}

private:

};

Application* CreateApplication()
{
	return new Sandbox();
}