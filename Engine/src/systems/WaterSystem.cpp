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
	poolQuad.addComponent<MeshComponent>(MeshCollection::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Meshes/sd_plane.fbx"));
	poolQuad.addComponent<RenderableComponent>();

	auto& planeTransform = poolQuad.getComponent<Transformation>();
	planeTransform.rotate({ 1,0,0 }, 90);
	planeTransform.scale({ 100, 100, 1 });

	auto& shader = Shader::createOverrideShader("WaterShader", SGE_ROOT_DIR + "Resources/Engine/Shaders/WaterShader.glsl", ShaderOverride::PBR);

	auto& shaderComponent = poolQuad.addComponent<ShaderComponent>();
	shaderComponent.setShader(shader);

	auto waterNormal = Texture::import(SGE_ROOT_DIR + "Resources/Engine/Textures/water_new_height.png");
	shaderComponent.addTexture("waterNormalSampler", waterNormal);

	return poolQuad;
}
