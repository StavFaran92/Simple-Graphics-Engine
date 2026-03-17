#pragma once

#include "core/Configurations.h"

template<typename T>
class ResourceWrapper;

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
};