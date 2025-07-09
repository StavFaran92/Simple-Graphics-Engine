#include "render/Graphics.h"

#include "core/Engine.h"

Graphics::Graphics()
{
	Engine::get()->registerSubSystem<Graphics>(this);
}