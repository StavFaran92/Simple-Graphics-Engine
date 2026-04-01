#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "geometry/Model.h"

struct EngineAPI MeshRendererComponent : public Component
{
	MeshRendererComponent(AssetHandle<ModelAsset> mesh);
	MeshRendererComponent() = default;

	void addMaterial(const AssetHandle<MaterialAsset>& mat)
	{
		m_material[count++] = mat;
	}

	void setMaterial(int index, const AssetHandle<MaterialAsset>& mat)
	{
		m_material[index] = mat;
	}

	AssetHandle<MaterialAsset> getMaterialBySlot(int slot) const;

	ResourceWrapper<Material> at(int index)
	{
		auto iter = m_material.find(index);
		if (iter == m_material.end())
		{
			return Engine::get()->getDefaultMaterial();
		}
		return iter->second.resource();
	}	

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(mesh);
		SERIALIZED_MEMBER(count);
		SERIALIZED_MEMBER(m_material);
		SERIALIZED_MEMBER(renderTechnique);

	}

	

	AssetHandle<ModelAsset> mesh = AssetHandle<ModelAsset>::empty;

	int count = 0;
	std::unordered_map<int, AssetHandle<MaterialAsset>> m_material;

	enum class RenderTechnique : int
	{
		Forward,
		Deferred
	};

	RenderTechnique renderTechnique = RenderTechnique::Deferred;

protected:
	std::vector<AssetHandle<Asset>*> gatherDependenciesInternal() const override;
};

REGISTER_COMPONENT(MeshRendererComponent)