#pragma once

#include "memory/ResourceWrapper.h"

struct ResourceLoadDescriptor;
struct ResourceCreateDescriptor;
struct AssetCreateDescriptor;
struct AssetRecord;
class Asset;

// Validates / fills defaults on resource descriptors
class IResourceParser
{
public:
	virtual ~IResourceParser() = default;
	virtual void parse(ResourceLoadDescriptor& desc) = 0;
	virtual void parse(ResourceCreateDescriptor& desc) = 0;
};

// Loads a resource from disk given a load descriptor
class IResourceLoader
{
public:
	virtual ~IResourceLoader() = default;
	virtual ResourceWrapper<Resource> loadFromDisk(ResourceLoadDescriptor& desc) = 0;
};

// Creates a resource procedurally given a create descriptor
//class IResourceFactory
//{
//public:
//	virtual ~IResourceFactory() = default;
//	virtual ResourceWrapper<Resource> create(ResourceCreateDescriptor& desc) = 0;
//};

// Saves an asset to disk
class IResourceSaver
{
public:
	virtual ~IResourceSaver() = default;
	virtual void save(const AssetRecord& record) = 0;
};

// Imports an asset from an external file into the project
class IResourceImporter
{
public:
	virtual ~IResourceImporter() = default;
	virtual void import(const AssetRecord& record) = 0;
};

// Creates a resource procedurally given a create descriptor
class IAssetFactory
{
public:
	virtual ~IAssetFactory() = default;
	virtual Asset* create(AssetCreateDescriptor& desc) = 0;
};

// Per-type manager that provides access to the pipeline stages
class ResourceTypeManager
{
public:
	virtual ~ResourceTypeManager() = default;
	virtual IResourceParser*   getParser() = 0;
	virtual IResourceLoader*   getLoader(const std::string& ext) = 0;
	//virtual IResourceFactory*  getFactory() = 0;
	virtual IResourceSaver*    getSaver(const std::string& ext) = 0;
	virtual IResourceImporter* getImporter(const std::string& ext) = 0;
	virtual IAssetFactory*     getAssetFactory() = 0;
};
