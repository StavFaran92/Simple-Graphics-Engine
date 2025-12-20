#include "EntryPoint.h"
#include "sge.h"


class CustomBoxBehaviour : public ScriptableEntity
{
	virtual void onUpdate(float dt) override
	{
		static float theta = 0;
		theta += .01f;
		auto& trans = entity.getComponent<Transformation>();
		trans.setLocalPosition({ cos(theta), 1, sin(theta) });
	}
};

class Sandbox : public Application
{
public:

	void start() override
	{
		Skybox::CreateSkyboxFromEquirectangularMap({ SGE_ROOT_DIR "Resources/Engine/Textures/Skybox/right.jpg",
		SGE_ROOT_DIR "Resources/Engine/Textures/Skybox/left.jpg",
		SGE_ROOT_DIR "Resources/Engine/Textures/Skybox/top.jpg",
		SGE_ROOT_DIR "Resources/Engine/Textures/Skybox/bottom.jpg",
		SGE_ROOT_DIR "Resources/Engine/Textures/Skybox/front.jpg",
		SGE_ROOT_DIR "Resources/Engine/Textures/Skybox/back.jpg" });


		{
			auto ground = ShapeFactory::createBox(Engine::get()->getContext()->getActiveScene().get());
			auto& groundTransfrom = ground.getComponent<Transformation>();
			groundTransfrom.setLocalScale({ 10, .1f, 10 });
			auto& mat = ground.addComponent<Material>();
			auto texDiff = Engine::get()->getSubSystem<Assets>()->importTexture2D("Resources/Content/Textures/brickwall.jpg");
			mat.setTexture(Texture::Type::Albedo, Resource<Texture>(texDiff));
			auto texNorm = Engine::get()->getSubSystem<Assets>()->importTexture2D("Resources/Content/Textures/brickwall_normal.jpg");
			mat.setTexture(Texture::Type::Normal, Resource<Texture>(texNorm));
		}

		auto pLight = getContext()->getActiveScene()->createEntity();
		pLight.addComponent<PointLight>(glm::vec3{ 1,1,1 }, 1.f, 1.f, Attenuation());
		pLight.addComponent<Transformation>(pLight, glm::vec3{ 0,2,0 });
		pLight.addComponent<NativeScriptComponent>().bind<CustomBoxBehaviour>();


		//postProcess(PostProcess::grayscale());
	}

};

Application* CreateApplication()
{
	return new Sandbox();
}