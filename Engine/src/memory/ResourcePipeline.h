#pragma once

#include "memory/AssetRecord.h"
#include "memory/AssetDescriptors.h"
#include "memory/ResourceWrapper.h"

class Asset;
class Resource;
struct AssetCreateDescriptor;
struct ResourceCreateDescriptor;
struct ResourceLoadDescriptor;

// Validates / fills defaults on resource descriptors
class ResourceTypeManager
{
public:
    // ============================================================
    //  Asset Creation
    // ============================================================

    // Create a new asset instance in memory
    virtual Asset* createAsset(AssetCreateDescriptor& desc) = 0;

    // Import asset into the engine
    virtual bool importAsset(const std::string& src, const ScopedPath& dst) = 0;

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
