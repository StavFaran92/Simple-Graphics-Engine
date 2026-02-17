#include "memory/ResourceBase.h"

#include "memory/ResourceWrapper.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetFactory.h"
#include "core/Logger.h"

ResourceWrapper<Resource> Resource::load(ResourceLoadDescriptor& desc)
{
	AssetType type = desc.aType;

	ResourceTypeManager* manager = AssetFactory::getManager(type);
	if (!manager)
	{
		logError("No ResourceTypeManager registered for asset type {}", static_cast<int>(type));
		return ResourceWrapper<Resource>::empty;
	}

	manager->parse(desc);
	ResourceWrapper<Resource> resource = manager->loadFromDisk(desc);
	return resource;
}

ResourceWrapper<Resource> Resource::create(ResourceCreateDescriptor& desc)
{
	AssetType type = desc.aType;

	ResourceTypeManager* manager = AssetFactory::getManager(type);
	if (!manager)
	{
		logError("No ResourceTypeManager registered for asset type {}", static_cast<int>(type));
		return ResourceWrapper<Resource>::empty;
	}

	manager->parse(desc);

	ResourceWrapper<Resource> resource = desc.createResource();
	if (resource.isEmpty())
	{
		logError("Failed to create resource of type {}", static_cast<int>(type));
		return ResourceWrapper<Resource>::empty;
	}

	return resource;
}
