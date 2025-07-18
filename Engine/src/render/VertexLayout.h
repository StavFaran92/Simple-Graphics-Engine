#pragma once

#include <vector>

#include "core/Core.h"
#include "core/Configurations.h"


struct EngineAPI VertexLayout
{
	void build()
	{
		// calculate stride
		for (auto entry : attribs)
		{
			auto& attribData = getAttributeData(entry);
			stride += attribData.length * attribData.size;
		}
	}

	std::vector<LayoutAttribute> attribs;
	size_t numOfVertices = 0;
	size_t stride = 0;
};
