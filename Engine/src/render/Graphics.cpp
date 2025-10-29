#include "render/Graphics.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "render/DeferredRenderer.h"

Graphics::Graphics()
{
	Engine::get()->registerSubSystem<Graphics>(this);
}

void Graphics::reloadDefferedRendererShaders()
{
	Engine::get()->getDeferredRenderer().reloadShaders();
}
