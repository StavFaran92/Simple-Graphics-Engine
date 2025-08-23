#include "MaterialComponent.h"
#include "render/Material.h"

MaterialComponent::MaterialComponent()
{
	auto mat = Engine::get()->getDefaultMaterial()->clone();

	//auto mat = std::make_shared<Material>(*Engine::get()->getDefaultMaterial().get());
	materials[0] = mat;
}