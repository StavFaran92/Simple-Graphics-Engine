#include "Asset.h"

#include "core/Logger.h"
#include "memory/Assets.h"
#include "core/Engine.h"
//
//Asset::Asset(const AssetBuildDescriptor& desc)
//	:m_createDesc (desc)
//{
//}

void Asset::updateAsset(AssetUpdateDescriptor& desc, ResourceBuildDescriptor& resourceDesc)
{
	Engine::get()->getSubSystem<Assets>()->updateAsset(uuid, desc, resourceDesc);
}
