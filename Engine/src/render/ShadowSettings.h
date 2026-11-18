#pragma once

#include <glm/glm.hpp>

// Global directional-light shadow settings, consumed by ShadowSystem and the lighting shaders.
struct ShadowSettings
{
	bool enabled = true;

	// Resolution of each cascade (square); changing it or the count reallocates the depth array
	int resolution = 2048;

	int cascadeCount = 4;

	// 0 = uniform splits, 1 = logarithmic; higher puts more resolution near the camera
	float splitLambda = 0.75f;

	// Shadows end here, independent of the camera's far plane
	float shadowDistance = 150.0f;

	// Light-space depth pushed toward the light so casters outside a slice still cast into it
	float casterMargin = 50.0f;

	// Depth bias in shadow-map texels, converted per cascade
	float biasScale = 1.5f;
};

// Must match MAX_SHADOW_CASCADES in Shadows.glsl
constexpr int MAX_SHADOW_CASCADES = 4;

struct ShadowCascades
{
	int count = 0;
	glm::mat4 lightSpaceMatrices[MAX_SHADOW_CASCADES];
	float splits[MAX_SHADOW_CASCADES] = {}; // far view-space distance of each cascade
	float bias[MAX_SHADOW_CASCADES] = {};   // in normalized shadow-map depth
};
