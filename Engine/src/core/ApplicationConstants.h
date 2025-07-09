#pragma once

#include <string>
#include "glm/glm.hpp"

namespace Constants {
	static const float PI = 3.14159265358979323846f;
	static const float toRadians = PI / 180;
	static const float toDegrees = 180 / PI;

	static const std::string g_textureDiffuse = "texture_diffuse";
	static const std::string g_textureSpecular = "texture_specular";
	static const std::string g_textureNormal = "samplerNormal";
	static const std::string g_textureAlbedo = "samplerAlbedo";
	static const std::string g_textureMetallic = "samplerMetallic";
	static const std::string g_textureRoughness = "samplerRoughness";
	static const std::string g_textureAO = "samplerAO";

	static const glm::vec3 VEC3_ONE = glm::vec3(1.0f, 1.0f, 1.0f);
	static const glm::vec3 VEC3_ZERO = glm::vec3(0.0f, 0.0f, 0.0f);

	static const char* g_supportedFormats[] = {
		"*.obj" ,"*.blend", "*.fbx", "*.dae", "*.gltf"
	};

	//const char* supportedImageFormats[4] = { "*.png", "*.jpg", "*.bmp", "*.tga" };

	static const char* g_textureSupportedFormats[] = {
		"*.png", "*.jpg", "*.bmp", "*.tga",
	};

	static const char* g_animationSupportedFormats[] = {
		"*.fbx",
	};

	



	static const std::string N_A = "N/A";
}