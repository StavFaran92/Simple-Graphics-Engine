#include "core/System.h"

#include "core/Engine.h"

System::System()
{
}

double System::getFPS() const
{
	return m_fps;
}

double System::getDeltaTime() const
{
	return m_deltaTime;
}

uint64_t System::getFrameCount() const
{
	return m_frameCount;
}

unsigned int System::getTriangleCount() const
{
	return m_triangleCount;
}

void System::tick(double deltaTime)
{
	m_fps = 1.0 / deltaTime;
	m_deltaTime = deltaTime;
	++m_frameCount;
}

void System::reset()
{
	m_triangleCount = 0;
}

void System::addTriangleCount(unsigned int count)
{
	m_triangleCount += count;
}
