#pragma once

#include "Core.h"

class EngineAPI System
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