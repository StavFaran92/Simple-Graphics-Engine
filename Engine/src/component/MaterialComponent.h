#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI MaterialComponent : public Component
{
	MaterialComponent();


	auto begin() { return materials.begin(); }
	auto end() { return materials.end(); }
	auto begin() const { return materials.begin(); }
	auto end() const { return materials.end(); }

	void addMaterial(const ResourceWrapper<Material>& mat)
	{
		materials[count++] = mat;
	}

	void setMaterial(int index, const ResourceWrapper<Material>& mat)
	{
		materials[index] = mat;
	}

	ResourceWrapper<Material> at(int index)
	{
		auto iter = materials.find(index);
		if (iter == materials.end())
		{
			return Engine::get()->getDefaultMaterial();
		}
		return iter->second;
	}

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene&)
	{
		attachSimple<MaterialComponent>(c, entityHandler);
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(materials);
		SERIALIZED_MEMBER(count);
	}

	std::map<int, ResourceWrapper<Material>> materials;
	int count = 0;

};

REGISTER_COMPONENT(MaterialComponent)