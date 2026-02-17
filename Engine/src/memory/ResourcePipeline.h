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
    //  Creation (Editor side)
    // ============================================================

    // Create a new asset instance in memory (e.g new Material, new Mesh)
    virtual Asset* createAsset(AssetCreateDescriptor& desc) = 0;

    // Called when user imports an external file (FBX/PNG/etc)
    virtual void importAsset(const AssetRecord& record) = 0;

    // Save asset metadata (.asset / .meta)
    virtual void saveAsset(const AssetRecord& record) = 0;


    // ============================================================
    //  Runtime resource loading
    // ============================================================

    // Create a load descriptor from asset record
    virtual ResourceLoadDescriptor* createLoadDescriptor(const AssetRecord& record) = 0;

    // Actually load the runtime resource (GPU buffers, textures, etc)
    virtual ResourceWrapper<Resource> loadFromDisk(ResourceLoadDescriptor& desc) = 0;


    // ============================================================
    //  Descriptor parsing
    // ============================================================

    // Parse serialized asset data -> runtime loading descriptor
    virtual void parse(ResourceLoadDescriptor& desc) = 0;

    // Parse asset data -> creation descriptor (editor creation)
    virtual void parse(ResourceCreateDescriptor& desc) = 0;

};
