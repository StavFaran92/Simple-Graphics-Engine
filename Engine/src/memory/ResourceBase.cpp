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

	// Parse the descriptor
	IResourceParser* parser = manager->getParser();
	if (parser)
	{
		parser->parse(desc);
	}

	// Load from disk
	std::string ext = desc.sourcePath.substr(desc.sourcePath.find_last_of('.'));
	IResourceLoader* loader = manager->getLoader(ext);
	if (!loader)
	{
		logError("No loader registered for extension '{}' on asset type {}", ext, static_cast<int>(type));
		return ResourceWrapper<Resource>::empty;
	}

	return loader->loadFromDisk(desc);
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

	// Parse the descriptor
	IResourceParser* parser = manager->getParser();
	if (parser)
	{
		parser->parse(desc);
	}

	ResourceWrapper<Resource> resource = desc.createResource();
	if (resource.isEmpty())
	{
		logError("Failed to create resource of type {}", static_cast<int>(type));
		return ResourceWrapper<Resource>::empty;
	}

	return resource;
}
