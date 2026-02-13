#include "EntryPoint.h"
#include "sge.h"

class CustomBoxBehaviour : public ScriptableEntity
{
	virtual void onUpdate(float dt) override
	{
		static float theta = 0;
		theta += .01f;
		auto& dirLight = entity.getComponent<DirectionalLight>();
		dirLight.setDirection({ cos(theta), -1, sin(theta) });
	}
};

class Sandbox : public Application
{
public:

	void start() override
	{
		//auto skybox = Skybox::CreateSkybox(Engine::get()->getContext()->getActiveScene().get());

		Skybox::CreateSkyboxFromEquirectangularMap({ SGE_ROOT_DIR "Resources/Engine/Textures/Skybox/right.jpg",
		SGE_ROOT_DIR"Resources/Engine/Textures/Skybox/left.jpg",
		SGE_ROOT_DIR"Resources/Engine/Textures/Skybox/top.jpg",
		SGE_ROOT_DIR"Resources/Engine/Textures/Skybox/bottom.jpg",
		SGE_ROOT_DIR"Resources/Engine/Textures/Skybox/front.jpg",
		SGE_ROOT_DIR"Resources/Engine/Textures/Skybox/back.jpg" });

		{
			auto box = ShapeFactory::createBox(Engine::get()->getContext()->getActiveScene().get());
			box.getComponent<Transformation>().translate({ 0, 2, 0 });
			auto& mat = box.addComponent<Material>();
			auto tex = Engine::get()->getSubSystem<Assets>()->importTexture2D("Resources/Content/Textures/checkers.jpg");
			mat.setTexture(Texture::Type::Albedo, Resource<Texture>(tex));
			//auto& nsc = box.addComponent<NativeScriptComponent>();
			//nsc.bind<CustomBoxBehaviour>();
		}


		{
			auto ground = ShapeFactory::createBox(Engine::get()->getContext()->getActiveScene().get());
			auto& groundTransfrom = ground.getComponent<Transformation>();
			groundTransfrom.setLocalScale({ 50, .5f, 50 });
			auto& mat = ground.addComponent<Material>();
			auto tex = Engine::get()->getSubSystem<Assets>()->importTexture2D("Resources/Content/Textures/floor.jpg");
			mat.setTexture(Texture::Type::Albedo, Resource<Texture>(tex));
		}

		auto entt = Engine::get()->getContext()->getActiveScene()->getRegistry().view<DirectionalLight>().front();
		Entity e{ entt, Engine::get()->getContext()->getActiveScene().get() };
		e.addComponent<NativeScriptComponent>().bind<CustomBoxBehaviour>();

		

		//postProcess(PostProcess::grayscale());
	}

};

Application* CreateApplication()
{
	return new Sandbox();
}