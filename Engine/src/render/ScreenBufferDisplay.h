#pragma once

#include <memory>

#include "runtime/Entity.h"
#include "memory/ResourceWrapper.h"

class FrameBufferObject;
class RenderBufferObject;
class Shader;
class ScreenQuad;
class IRenderer;
class TextureHandler;
class Scene;

class ScreenBufferDisplay
{
public:
	ScreenBufferDisplay(Scene* scene);
	bool init(int windowWidth, int windowHeight);
	void draw(ResourceWrapper<Texture> textureHandler);

private:
	Entity m_quad;
	ResourceWrapper<Shader> m_screenShader = nullptr;
	std::shared_ptr<IRenderer> m_renderer = nullptr;
	Scene* m_scene = nullptr;
};

