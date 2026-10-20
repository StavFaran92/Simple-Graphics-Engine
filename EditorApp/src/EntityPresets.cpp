#include "EntityPresets.h"

namespace
{
	SGE_Regsitry* activeRegistry()
	{
		return &Engine::get()->getContext()->getActiveScene()->getRegistry();
	}

	Entity createNamed(const std::string& name)
	{
		return Engine::get()->getContext()->getActiveScene()->createEntity(name);
	}
}

std::string EntityPreset::name() const
{
	return path.substr(path.find_last_of('/') + 1);
}

const std::vector<EntityPreset>& entityPresets()
{
	static const std::vector<EntityPreset> presets = {
		{ "Entity", "Empty entity with only a name and a Transformation",
			[] {
				static int count = 0;
				return createNamed("Entity_" + std::to_string(count++));
			} },

		{ "Primitive/Cube", "1x1x1 box mesh with the default material",
			[] { return ShapeFactory::createBoxEntity(activeRegistry()); } },
		{ "Primitive/Sphere", "Sphere mesh of radius 1 with the default material",
			[] { return ShapeFactory::createSphere(activeRegistry()); } },
		{ "Primitive/Quad", "2x2 quad mesh in the XY plane facing +Z, with the default material",
			[] { return ShapeFactory::createQuad(activeRegistry()); } },
		{ "Primitive/Cylinder", "Cylinder mesh of height 1 and radius 1 with the default material",
			[] { return ShapeFactory::createCylinder(activeRegistry()); } },

		{ "Light/Directional Light", "Entity with a DirectionalLight pointing down",
			[] {
				static int count = 0;
				Entity e = createNamed("Directional_Light_" + std::to_string(count++));
				// The first argument is the color; black made the light do nothing
				e.addComponent<DirectionalLight>(glm::vec3{ 1,1,1 }, glm::vec3{ 0,-1,0 }, 1.f, 1.f);
				return e;
			} },
		{ "Light/Point Light", "Entity with a PointLight and default attenuation",
			[] {
				static int count = 0;
				Entity e = createNamed("Point_Light_" + std::to_string(count++));
				e.addComponent<PointLight>(glm::vec3{ 0,0,0 }, 1.f, 1.f, Attenuation());
				return e;
			} },

		{ "Environment/Skybox", "Skybox and image-based lighting from the built-in sunflowers HDR; takes a few seconds",
			[] { return Skybox::createSkybox(SGE_ROOT_DIR "Resources/Engine/Textures/sunflowers_puresky_4k.hdr", Skybox::TexType::EQUIRECTANGULAR); } },
		{ "Environment/Terrain", "1000x1000 flat terrain with the default checkerboard terrain material",
			[] { return Terrain::createTerrain(1000, 1000); } },
		{ "Environment/Pool", "Water body: a 'Pool' entity with a WaterBodyComponent and a 10x10 water plane child",
			[] { return WaterSystem::createPool(); } },
		{ "Environment/Clouds", "Volumetric clouds: a parent with a VolumetricCloudsComponent and a box 'Volume' child",
			[] { return VolumetricCloudsSystem::createVolumetricClouds(); } },
	};
	return presets;
}

const EntityPreset* findPreset(const std::string& name)
{
	for (const auto& preset : entityPresets())
	{
		if (preset.name() == name)
			return &preset;
	}
	return nullptr;
}
