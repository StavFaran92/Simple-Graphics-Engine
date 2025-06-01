#include "EntryPoint.h"
#include "sge.h"

#include "CameraScript.h"

class PostProcessSample : public Application
{
public:

	void start() override
	{
		auto ent = Engine::get()->getContext()->getActiveScene()->createEntity();
		ent.addComponent<VolumeComponent>();

		auto& shader = Shader::createOverrideShader("RayMarchSample", "../../PostProcessSample/Resources/Content/Shaders/BasicShader.glsl", ShaderOverride::Volume);
		auto& shaderComponent = ent.addComponent<ShaderComponent>();
		shaderComponent.setShader(shader);

		ShapeFactory::createSphere(&Engine::get()->getContext()->getActiveScene()->getRegistry());

		auto editorCamera = Engine::get()->getContext()->getActiveScene()->createEntity("Editor Camera");
		editorCamera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)4 / 3, 0.1f, 3000.0f));
		editorCamera.addComponent<NativeScriptComponent>().bind<CameraScript>();
		Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(editorCamera);

		Engine::get()->getContext()->getActiveScene()->startSimulation();
	}

};

Application* CreateApplication()
{
	return new PostProcessSample();
}