#pragma once

#include "core/Configurations.h"

template<typename T>
class ResourceRef;

class Resource
{
public:
	Resource() = default;
	virtual ~Resource() = default;

	// Runtime type of the resource for asset baking
	// Default: NONE; derived classes should override
	virtual AssetType getType() const { return AssetType::NONE; }

	static ResourceID getNewResourceID()
	{
		return resourceCounter++;
	}

	inline static ResourceID resourceCounter = 1;
};