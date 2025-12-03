#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI MeshRendererComponent : public Component
{
	MeshRendererComponent(AssetWrapper<MeshCollection> mesh);
	MeshRendererComponent() = default;

	void addMaterial(const AssetWrapper<Material>& mat)
	{
		m_material[count++] = mat;
	}

	void setMaterial(int index, const AssetWrapper<Material>& mat)
	{
		m_material[index] = mat;
	}

	AssetWrapper<Material> getMaterialBySlot(int slot) const;

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
		SERIALIZED_MEMBER(renderTechnique);

	}

	AssetWrapper<MeshCollection> mesh = AssetWrapper<MeshCollection>::empty;

	int count = 0;
	std::unordered_map<int, AssetWrapper<Material>> m_material;

	enum class RenderTechnique : int
	{
		Forward,
		Deferred
	};

	RenderTechnique renderTechnique = RenderTechnique::Deferred;

	
};

REGISTER_COMPONENT(MeshRendererComponent)