#pragma once

#include "Frustum.h"

class Volume
{
public:
	virtual bool isOnFrustum(const Frustum& frustum) const = 0;
};