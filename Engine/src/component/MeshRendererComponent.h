#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI MeshRendererComponent : public Component
{
	MeshRendererComponent(AssetWrapper<MeshCollection> mesh) : mesh(mesh) {};
	MeshRendererComponent() = default;

	void addMaterial(const AssetWrapper<Material>& mat)
	{
		m_material[count++] = mat;
	}

	void setMaterial(int index, const AssetWrapper<Material>& mat)
	{
		m_material[index] = mat;
	}

	ResourceWrapper<Material> at(int index)
	{
		auto iter = m_material.find(index);
		if (iter == m_material.end())
		{
			return Engine::get()->getDefaultMaterial();
		}
		return iter->second.resource();
	}

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene&)
	{
		attachSimple<MeshRendererComponent>(c, entityHandler);
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(mesh);
		SERIALIZED_MEMBER(count);
		SERIALIZED_MEMBER(m_material);

	}

	AssetWrapper<MeshCollection> mesh = AssetWrapper<MeshCollection>::empty;

	int count = 0;
	std::unordered_map<int, AssetWrapper<Material>> m_material;
	
};

REGISTER_COMPONENT(MeshRendererComponent)