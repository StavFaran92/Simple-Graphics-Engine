#include "Graphics.h"

#include "Engine.h"

Graphics::Graphics()
{
	Engine::get()->registerSubSystem<Graphics>(this);
}