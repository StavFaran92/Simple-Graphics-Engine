#pragma once

#include "memory/AssetRecord.h"
#include "memory/AssetDescriptors.h"
#include "memory/ResourceWrapper.h"
#include <map>
#include <string>

class Asset;
class Resource;
struct AssetCreateDescriptor;
struct ResourceCreateDescriptor;
struct ResourceLoadDescriptor;

enum class CreationType
{
    Create,
    Import
};

struct ImportNode
{
	std::string name;
	std::map<std::string, ImportNode> dependencies;
	AssetCreateDescriptor createDescriptor;
    CreationType creationType = CreationType::Create;
    int index = 0;
};

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename... Args>
Ref<T> createRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// Validates / fills defaults on resource descriptors
class ResourceTypeManager
{
public:
    // ============================================================
    //  Asset Creation
    // ============================================================

    // Create a new asset instance in memory
    virtual Ref<Asset> createAsset(AssetCreateDescriptor& desc) = 0;

    // Import asset into the engine
    virtual bool importAsset(const std::string& src, ImportNode& result) = 0;

    // Save resource data on disk
    virtual bool saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst) = 0;

    // Load asset metadata (.asset / .meta)
    //virtual AssetRecord loadAsset(UUID uid) = 0; ???


    // ============================================================
    //  Runtime resource loading
    // ============================================================

    // Create load descriptor
    virtual ResourceLoadDescriptor* makeResourceLoadDescriptor() = 0;

    // load the runtime resource
    virtual ResourceWrapper<Resource> loadResourceFromDisk(ResourceLoadDescriptor& desc) = 0;


    // ============================================================
    //  Descriptor parsing
    // ============================================================

    // Parse serialized asset data -> runtime loading descriptor
    virtual void parse(ResourceLoadDescriptor& desc) = 0;

    // Parse asset data -> creation descriptor (editor creation)
    virtual void parse(ResourceCreateDescriptor& desc) = 0;

};
