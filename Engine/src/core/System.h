#pragma once

#include <cstdint>

#include "core/Core.h"

#include "systems/SubSystem.h"

class EngineAPI System : public SubSystem
{
public:
	System();

	double getFPS() const;
	double getDeltaTime() const;
	uint64_t getFrameCount() const;
	unsigned int getTriangleCount() const;

	void tick(double fps);
	void reset();
	void addTriangleCount(unsigned int count);

private:
	double m_fps = 0;
	double m_deltaTime = 0;

	uint64_t m_frameCount = 0;
	unsigned int m_triangleCount = 0;
};