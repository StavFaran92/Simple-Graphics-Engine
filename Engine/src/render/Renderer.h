#pragma once
#include "render/IRenderer.h"

#include "core/Core.h"
#include "memory/ResourceWrapper.h"

// forward declerations
class ICamera;
class SkyboxRenderer;
class Model;
class Mesh;
class Transformation;
class Shader;
class Mesh;
class Scene;


class Renderer : public IRenderer
{
public:
	// Constructor
	bool init();

	void renderSceneUsingCustomShader(Scene* scene);

	void enableWireframeMode(bool enable);

	void renderScene(Scene*);

	void renderSceneNonOpaque(Scene*);

private:
	void setUniforms();
protected:
	bool m_wireFrameMode = false;

	//ResourceWrapper<Shader> m_pbrShader = nullptr;
	ResourceWrapper<MeshCollection> m_quad = nullptr;
};
