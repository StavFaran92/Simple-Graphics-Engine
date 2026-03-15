#include "Asset.h"

#include "core/Logger.h"
#include "memory/Assets.h"
#include "core/Engine.h"
//
//Asset::Asset(const AssetCreateDescriptor& desc)
//	:m_createDesc (desc)
//{
//}

void Asset::updateAsset(AssetUpdateDescriptor desc)
{
	Engine::get()->getSubSystem<Assets>()->updateAsset(uuid, desc);
}
