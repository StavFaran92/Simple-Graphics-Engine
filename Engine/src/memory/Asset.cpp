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