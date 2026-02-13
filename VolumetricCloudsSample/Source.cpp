#include "EntryPoint.h"
#include "sge.h"

#include "CameraScript.h"

class VolumetricCloudsSample : public Application
{
public:

	void start() override
	{
		auto ent = Engine::get()->getContext()->getActiveScene()->createEntity();
		auto& volume = ent.addComponent<VolumeComponent>();

		auto& shader = Shader::createOverrideShader("../../VolumetricCloudsSample/Resources/Content/Shaders/BasicShader.glsl", ShaderOverride::Volume, true);

		MaterialImportSettings shaderDesc;
		shaderDesc.aType = AssetType::SHADER;
		shaderDesc.name = "volumeShader";
		shaderDesc.origFilePath = "../../VolumetricCloudsSample/Resources/Content/Shaders/BasicShader.glsl";
		AssetWrapper<Shader> shaderAsset = Engine::get()->getSubSystem<Assets>()->createAsset(shader, shaderDesc).as<Shader>();

		ResourceWrapper<Material> material = Material::create(MaterialRenderMode::Custom);
		material->setCustomShader(shaderAsset);

		MaterialImportSettings desc;
		desc.aType = AssetType::MATERIAL;
		desc.name = "VolumeMat";
		AssetWrapper<Material> materialAsset = Engine::get()->getSubSystem<Assets>()->createAsset(material, desc).as<Material>();

		volume.material = materialAsset;
		volume.mesh = BuiltInAssets::getByName<MeshCollection>(SGE_MESH_BOX);

		ShapeFactory::createSphere(&Engine::get()->getContext()->getActiveScene()->getRegistry());

		auto editorCamera = Engine::get()->getContext()->getActiveScene()->createEntity("Editor Camera");
		editorCamera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)4 / 3, 0.1f, 3000.0f));
		editorCamera.addComponent<NativeScriptComponent>().bind<CameraScript>();
		Engine::get()->getContext()->getActiveScene()->setGameCamera(editorCamera);

		Engine::get()->getContext()->getActiveScene()->startSimulation();

	}

};

Application* CreateApplication()
{
	return new VolumetricCloudsSample();
}