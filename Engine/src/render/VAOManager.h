#pragma once

#include "core/Core.h"
#include "systems/SubSystem.h"
#include "render/GigaVAO.h"
#include "render/VertexArrayObject.h"

class EngineAPI VAOManager : public SubSystem
{
public:
	enum class Type
	{
		StaticGeometry,
		SkinnedGeometry
	};

	GigaVAO& getGigaVAO(Type type);
	VertexArrayObject& getQuadVAO();
	VertexArrayObject& getBoxVAO();

	VAOManager();
	~VAOManager() = default;

private:
	GigaVAO m_staticGeometry;
	GigaVAO m_skinnedGeometry;
};
