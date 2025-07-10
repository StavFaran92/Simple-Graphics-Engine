#pragma once

#include "IRenderer.h"

#include "render/VertexArrayObject.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "Resource.h"
#include "Entity.h"

class EngineAPI RendererIntermediate : public IRenderer
{
public:
	void clear() const override;
protected:
	void draw(const VertexArrayObject& vao) const override;


	//bool setupRenderTarget(Scene* scene);

protected:
	//FrameBufferObject m_renderTargetFBO;
	//RenderBufferObject m_renderTargetRBO;
	//Resource<Texture> m_renderTargetTexture = nullptr;
	//Entity m_quad;
	//std::shared_ptr<IRenderer> m_2DRenderer;
	//Resource<Shader> m_screenShader;
};

