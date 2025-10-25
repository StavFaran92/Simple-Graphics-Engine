#pragma once

#include "core/Core.h"

#include "systems/SubSystem.h"

class EngineAPI System : public SubSystem
{
public:
	System();

	double getFPS() const;
	double getDeltaTime() const;
	unsigned int getTriangleCount() const;

	void setDeltaTime(double fps);
	void reset();
	void addTriangleCount(unsigned int count);

private:
	double m_fps = 0;
	double m_deltaTime = 0;

	unsigned int m_triangleCount = 0;
};