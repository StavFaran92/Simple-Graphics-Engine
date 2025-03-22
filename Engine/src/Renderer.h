#pragma once
#include "RendererIntermediate.h"

#include "Core.h"

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

	std::shared_ptr<Shader> m_pbrShader = nullptr;
	Entity m_quad;


	// Inherited via RendererIntermediate
	void render() override;

};
