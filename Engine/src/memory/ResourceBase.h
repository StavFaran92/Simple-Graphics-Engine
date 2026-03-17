#pragma once

#include "core/Configurations.h"

template<typename T>
class ResourceWrapper;

struct ResourceLoadDescriptor;
struct ResourceBuildDescriptor;

class Resource
{
public:
	Resource() = default;
	virtual ~Resource() = default;

	static ResourceID getNewResourceID()
	{
		return resourceCounter++;
	}

	inline static ResourceID resourceCounter = 1;

	// Unified pipeline entry points (delegate to ResourceTypeManager registry)
	static ResourceWrapper<Resource> load(ResourceLoadDescriptor& desc);
	static ResourceWrapper<Resource> create(ResourceBuildDescriptor& desc);
};