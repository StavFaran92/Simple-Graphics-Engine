#include "MeshRendererComponent.h"

#include "memory/BuiltInAssets.h"
#include "core/Logger.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"

MeshRendererComponent::MeshRendererComponent(AssetHandle<ModelAsset> mesh)
	: mesh(mesh)
{
	static auto onChangedCB = [this](UUID)
	{
			Engine::get()->getContext()->getActiveScene()->makeDirty();
	};
	mesh.registerOnChanged(onChangedCB);
	auto scene = Engine::get()->getContext()->getActiveScene();
	if (!scene.isEmpty()) // Hack
	{
		scene->makeDirty();
	}

	int materialCount = mesh.resource()->getMaterialCount();
	for (int i = 0; i < materialCount; i++)
	{
		addMaterial(BuiltInAssets::getByName<MaterialAsset>(SGE_MATERIAL_DEFAULT));
	}
}

AssetHandle<MaterialAsset> MeshRendererComponent::getMaterialBySlot(int slot) const
{
	auto iter = m_material.find(slot);
	if (iter == m_material.end())
	{
		logWarning("Could not find material in slot {}", std::to_string(slot));
		return AssetHandle<MaterialAsset>::empty;
	}
	return iter->second;
}

void MeshRendererComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, ResourceWrapper<Scene>&)
{
	
		if (auto tc = std::dynamic_pointer_cast<MeshRendererComponent>(c))
		{
			entityHandler.addComponent<MeshRendererComponent>(*tc);

			static auto onChangedCB = [tc](UUID)
				{
					Engine::get()->getContext()->getActiveScene()->makeDirty();
				};
			tc->mesh.registerOnChanged(onChangedCB);
			auto scene = Engine::get()->getContext()->getActiveScene();
			if (!scene.isEmpty()) // Hack
			{
				scene->makeDirty();
			}
		}

		//attachSimple<MeshRendererComponent>(c, entityHandler);
	
}
