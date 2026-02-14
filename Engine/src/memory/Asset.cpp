#include "Asset.h"

#include "core/Logger.h"
#include "memory/Assets.h"
#include "core/Engine.h"

Asset::Asset(const AssetCreateDescriptor& desc)
	:m_createDesc (desc)
{
}

const AssetCreateDescriptor& Asset::getDescriptor() const
{
	return m_createDesc;
}

UUID Asset::getUUID() const
{
	return m_uuid;
}

void Asset::updateAsset(const AssetUpdateDescriptor& uDesc)
{
	AssetRecord aInfo = Engine::get()->getSubSystem<Assets>()->getAsset(m_uuid).info();
	aInfo.update(uDesc);

	// Save asset
	save(aInfo);

	Engine::get()->getSubSystem<Assets>()->updateAsset(aInfo);
}
