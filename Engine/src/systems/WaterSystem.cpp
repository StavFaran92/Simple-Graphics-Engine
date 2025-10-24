#include "systems/WaterSystem.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "geometry/ModelImporter.h"
#include "component/MeshComponent.h"
#include "component/ShaderComponent.h"
#include "component/RenderableComponent.h"

Entity WaterSystem::createPool()
{
	auto poolQuad = Engine::get()->getContext()->getActiveScene()->createEntity("Pool");

	// TODO use grid instead
	ModelImportSettings aDesc;
	aDesc.isEngineOwned = true;
	poolQuad.addComponent<MeshComponent>(MeshCollection::import(SGE_ROOT_DIR + "Resources/Engine/Meshes/sd_plane.fbx", aDesc));
	poolQuad.addComponent<RenderableComponent>();

	auto& planeTransform = poolQuad.getComponent<Transformation>();
	planeTransform.rotate({ 1,0,0 }, 90);
	planeTransform.scale({ 100, 100, 1 });

	auto& shader = Shader::createOverrideShader("WaterShader", SGE_ROOT_DIR + "Resources/Engine/Shaders/WaterShader.glsl", ShaderOverride::PBR);

	AssetCreateDescriptor desc;
	desc.aType = AssetType::SHADER;
	desc.origFilePath = SGE_ROOT_DIR + "Resources/Engine/Shaders/WaterShader.glsl";
	desc.name = "WaterShader";
	desc.attributes["shader_override"] = Shader::getShaderOverrideAsStr(ShaderOverride::PBR);
	desc.isEngineOwned = true;
	auto& shaderAsset = Engine::get()->getSubSystem<Assets>()->createAsset(shader, desc).as<Shader>();

	auto& shaderComponent = poolQuad.addComponent<ShaderComponent>();
	shaderComponent.setShader(shaderAsset);

	auto waterNormal = Texture::import(SGE_ROOT_DIR + "Resources/Engine/Textures/water_new_height.png");
	shaderComponent.addTexture("waterNormalSampler", waterNormal);

	return poolQuad;
}
