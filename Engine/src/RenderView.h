#pragma once

#include "Entity.h"
#include "FrameBufferObject.h"
#include "RenderBufferObject.h"
#include "Texture.h"
#include "Resource.h"

class RenderView
{
public:
	struct Viewport
	{
		int x, y, w, h;
	};

	RenderView(Viewport viewport, const Entity& camera);

	Viewport getViewport() const;

	const Entity& getCamera() const;

	void setCamera(const Entity& camera);

	unsigned int getRenderTargetTextureID() const;

	unsigned int getRenderTargetFrameBufferID() const;

	void setTexture(Resource<Texture> texture);

	void bind();
	void unbind();

	//void setRenderTargetID(unsigned int targetID);

private:
	Viewport m_viewport;
	Entity  m_camera = Entity::EmptyEntity;

	std::shared_ptr<FrameBufferObject> m_renderTargetFBO;
	std::shared_ptr<RenderBufferObject> m_renderTargetRBO;
	Resource<Texture> m_renderTargetTexture = Resource<Texture>::empty;
};