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


class EngineAPI Renderer : public RendererIntermediate
{
public:
	// Constructor
	Renderer(Scene* scene);

	bool init() override;

	void renderSceneUsingCustomShader(Scene* scene);

	void enableWireframeMode(bool enable);
	// Inherited via IRenderer
	void renderScene(Scene*) override;

private:
	void setUniforms();
protected:
	bool m_wireFrameMode = false;

	Scene* m_scene = nullptr;

	Resource<Shader> m_pbrShader = nullptr;
	Entity m_quad;


	// Inherited via RendererIntermediate
	void render() override;

};
