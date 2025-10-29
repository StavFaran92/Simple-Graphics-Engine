#pragma once
#include "render/RendererIntermediate.h"

#include "core/Core.h"

// forward declerations
class ICamera;
class SkyboxRenderer;
class Model;
class Mesh;
class Transformation;
class Shader;
class Entity;
class Mesh;


class Renderer : public RendererIntermediate
{
public:
	// Constructor
	bool init() override;

	void renderSceneUsingCustomShader(Scene* scene);

	void enableWireframeMode(bool enable);
	// Inherited via IRenderer
	void renderScene(Scene*) override;

	void renderSceneNonOpaque(Scene*);

private:
	void setUniforms();
protected:
	bool m_wireFrameMode = false;

	ResourceWrapper<Shader> m_pbrShader = nullptr;
	Entity m_quad;


	// Inherited via RendererIntermediate
	void render() override;

};
