#include "AssetRef.h"

#include "memory/ResourceRef.h"
#include "memory/AssetRecord.h"
#include "core/Engine.h"
#include "memory/ResourceManager.h"
#include "memory/AssetFactory.h"
#include "memory/Asset.h"

const AssetRecord& AssetHandleImpl::getInfo(UUID uuid)
{
    return Engine::get()->getSubSystem<Assets>()->getInfo(uuid);
}