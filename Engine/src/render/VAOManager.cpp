#include "render/VAOManager.h"

#include "core/Engine.h"

VAOManager::VAOManager()
{
	Engine::get()->registerSubSystem<VAOManager>(this);
}
