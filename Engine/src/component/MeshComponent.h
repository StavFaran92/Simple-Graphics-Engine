#pragma once

//#include "component/Component.h"
//#include "component/ComponentSerializer.h"
//
//struct EngineAPI MeshRendererComponent : public Component
//{
//	MeshRendererComponent(const AssetHandle<ModelAsset>& mesh) : mesh(mesh) {};
//	MeshRendererComponent() = default;
//
//	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene&)
//	{
//		attachSimple<MeshRendererComponent>(c, entityHandler);
//	}
//
//	template <class Archive>
//	void serialize(Archive& archive) {
//		SERIALIZED_MEMBER(mesh);
//		SERIALIZED_MEMBER(materialSlot);
//
//	}
//
//	float materialSlot = 0; // todo this will be used (probably as a list) to support multi material models
//	AssetHandle<ModelAsset> mesh = AssetHandle<ModelAsset>::empty;
//	
//};
//
//REGISTER_COMPONENT(MeshRendererComponent)