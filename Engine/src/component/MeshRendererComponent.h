#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "geometry/Model.h"
#include "core/Engine.h"

struct EngineAPI MeshRendererComponent : public Component
{
	MeshRendererComponent(ModelAssetRef mesh);
	MeshRendererComponent() = default;

	std::string getName() override { return "MeshRendererComponent"; }

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
		SERIALIZE_COMPONENT_BASE;
		SERIALIZED_MEMBER(mesh);
		SERIALIZED_MEMBER(count);
		SERIALIZED_MEMBER(m_material);
		SERIALIZED_MEMBER(renderTechnique);
		SERIALIZED_MEMBER_OPTIONAL(isInstanced);

	}

	

	ModelAssetRef mesh = ModelAssetRef::empty;

	int count = 0;
	std::unordered_map<int, MaterialAssetRef> m_material;

	enum class RenderTechnique : int
	{
		Forward,
		Deferred,
		Indirect
	};

	RenderTechnique renderTechnique = RenderTechnique::Deferred;

	bool isInstanced = false;

protected:
	std::vector<AssetRef<Asset>> gatherDependenciesInternal() const override;
};

REGISTER_COMPONENT(MeshRendererComponent)