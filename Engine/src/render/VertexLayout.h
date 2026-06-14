#pragma once

#include <vector>

#include "core/Core.h"
#include "core/Configurations.h"


struct EngineAPI VertexLayout
{
	static size_t getStride(const VertexLayout& layout)
	{
		size_t stride = 0;

		for (auto entry : layout.attribs)
		{
			auto& attribData = getAttributeData(entry);
			stride += attribData.length * attribData.size;
		}

		return stride;
	}

	size_t getStride() const
	{
		return getStride(*this);
	}

	std::vector<LayoutAttribute> attribs;
	size_t numOfVertices = 0;
};
