#include "WaterSystem.h"

#include "core/Engine.h"
#include "Context.h"
#include "Scene.h"
#include "ModelImporter.h"

Entity WaterSystem::createPool()
{
	auto poolQuad = Engine::get()->getContext()->getActiveScene()->createEntity("Pool");

	// TODO use grid instead
	auto quadModel = Engine::get()->getSubSystem<ModelImporter>()->import(SGE_ROOT_DIR + "Resources/Engine/Meshes/sd_plane.fbx"); 

	poolQuad.addComponent<MeshComponent>(quadModel.mesh);
	poolQuad.addComponent<RenderableComponent>();

	auto& planeTransform = poolQuad.getComponent<Transformation>();
	planeTransform.rotate({ 1,0,0 }, 90);
	planeTransform.scale({ 100, 100, 1 });

	auto& shader = Shader::createOverrideShader("WaterShader", SGE_ROOT_DIR + "Resources/Engine/Shaders/WaterShader.glsl", ShaderOverride::PBR);

	auto& shaderComponent = poolQuad.addComponent<ShaderComponent>();
	shaderComponent.setShader(shader);

	auto waterNormal = Texture::importTexture2D(SGE_ROOT_DIR + "Resources/Engine/Textures/water_new_height.png");
	shaderComponent.addTexture("waterNormalSampler", waterNormal);

	return poolQuad;
}
