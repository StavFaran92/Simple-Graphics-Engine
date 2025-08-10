#pragma once

#include "runtime/Entity.h"
#include "texture/Texture.h"
#include "memory/Resource.h"

class FrameBufferObject;
class RenderBufferObject;

struct Viewport
{
	int x, y, w, h;
};

class RenderTarget
{
public:
	RenderTarget() = default;
	RenderTarget(Viewport viewport);

	std::shared_ptr<FrameBufferObject> m_renderTargetFBO;
	std::shared_ptr<RenderBufferObject> m_renderTargetRBO;
	Resource<Texture> m_renderTargetTexture = Resource<Texture>::empty;
};

class RenderView
{
public:
	

	RenderView(Viewport viewport, const Entity& camera, const std::string& name = "temp");

	Viewport getViewport() const;

	const Entity& getCamera() const;

	void setCamera(const Entity& camera);

	unsigned int getRenderTargetTextureID() const;
	Resource<Texture> getRenderTargetTexture() const;

        unsigned int getRenderTargetFrameBufferID() const;

        void setTexture(Resource<Texture> texture);

        void resize(int w, int h);

        void bind();
        void unbind();

	void swapToAdditionalTarget();
	void swapBackToMainTarget();

	std::string getName() const;

	//void setRenderTargetID(unsigned int targetID);

private:
	Viewport m_viewport;
	Entity  m_camera = Entity::EmptyEntity;

	RenderTarget renderTargets[2];

	unsigned int m_boundTargetTextureSlot = 0;

	const std::string m_name;
};