#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "geometry/Model.h"
#include "core/Engine.h"

struct EngineAPI MeshRendererComponent : public Component
{
	MeshRendererComponent(ModelAssetRef mesh);
	MeshRendererComponent() = default;

	void addMaterial(const MaterialAssetRef& mat)
	{
		m_material[count++] = mat;
	}

	void setMaterial(int index, const MaterialAssetRef& mat)
	{
		m_material[index] = mat;
	}

	MaterialAssetRef getMaterialBySlot(int slot) const;

	MaterialResourceRef at(int index)
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

	

	ModelAssetRef mesh = ModelAssetRef::empty;

	int count = 0;
	std::unordered_map<int, MaterialAssetRef> m_material;

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