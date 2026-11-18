#pragma once

#include <vector>

#include "core/Core.h"

#include "render/FrameBufferObject.h"
#include "render/ShadowSettings.h"
#include "memory/AssetAliases.h"

struct CameraComponent;
class Shader;
class Scene;
struct DrawItem;
class Texture;
class Context;

class EngineAPI ShadowSystem
{
public:
	enum class ShadowAlgorithm
	{
		ShadowMapping,
		CSM
	};

	ShadowSystem();

	bool init();

	// view and camera are the render view's; cascades are fitted to its frustum
	void renderToDepthMap(Scene* scene, const std::vector<DrawItem>& drawItems,
		const glm::mat4& view, const CameraComponent& camera);

	TextureResourceRef getShadowMap() const;
	const ShadowCascades& getCascades() const;

	// Sets the cascade uniforms read by Shadows.glsl, from Graphics::shadowCascades
	static void setCascadeUniforms(Shader& shader);
private:
	// Allocates the depth array from Graphics::shadowSettings
	bool createDepthMap();

	// Fills m_cascades.splits between znear and shadowDistance
	void computeSplits(float znear);

	// Fits cascade i to the frustum slice [sliceNear, sliceFar] and fills its matrix and bias
	void computeCascade(int i, float sliceNear, float sliceFar, const glm::mat4& view,
		const CameraComponent& camera, const glm::vec3& lightDir, const glm::vec3& lightUp);

	void drawCasters(Scene* scene, const std::vector<DrawItem>& drawItems);

private:
	bool m_isInit = false;
	int m_resolution = 0;

	ShadowAlgorithm m_shadowAlgorithm = ShadowAlgorithm::CSM;

	FrameBufferObject m_fbo;
	ShaderResourceRef m_simpleDepthShader = nullptr;
	TextureResourceRef m_depthMapTexture = nullptr;

	ShadowCascades m_cascades;
};