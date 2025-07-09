#include "System.h"

#include "Engine.h"

System::System()
{
	Engine::get()->registerSubSystem<System>(this);
}

double System::getFPS() const
{
	return m_fps;
}

double System::getDeltaTime() const
{
	return m_deltaTime;
}

unsigned int System::getTriangleCount() const
{
	return m_triangleCount;
}

void System::setDeltaTime(double deltaTime)
{
	m_fps = 1.0 / deltaTime;
	m_deltaTime = deltaTime;
}

void System::reset()
{
	m_triangleCount = 0;
}

void System::addTriangleCount(unsigned int count)
{
	m_triangleCount += count;
}
