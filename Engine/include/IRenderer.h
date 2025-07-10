#pragma once

// Includes
//STL
#include <memory>

#include "glm/glm.hpp"
#include "Core.h"
#include <entt/entt.hpp>
#include "Resource.h"

class Shader;
class VertexArrayObject;
class Model;
class ICamera;
class Transformation;
class Mesh;
class Entity;
class Scene;
class Context;
class ICamera;
class Material;
class TextureHandler;
class Frustum;

class EngineAPI IRenderer {
public:
	//struct DrawQueueRenderParams
	//{
	//	Scene* scene = nullptr;
	//	Context* context = nullptr;
	//	IRenderer* renderer = nullptr;
	//	entt::registry* registry = nullptr;
	//	glm::vec3 cameraPos;
	//	
	//	std::vector<Entity>* entityGroup;

	//	Entity* entity = nullptr;
	//	Mesh* mesh = nullptr;
	//	Shader* shader = nullptr;
	//	Material* material = nullptr;

	//	// MVP
	//	glm::mat4* model = nullptr;
	//	glm::mat4* view = nullptr;
	//	glm::mat4* projection = nullptr;

	//	Resource<Texture> irradianceMap = nullptr;
	//	Resource<Texture> prefilterEnvMap = nullptr;
	//	Resource<Texture> brdfLUT = nullptr;
	//	Resource<Texture> shadowMap = nullptr;
	//	glm::mat4 lightSpaceMatrix;
	//	Frustum* frustum = nullptr;
	//};
	enum class DrawType {
		Lines,
		Triangles
	};
public:
	IRenderer() {
		SetDrawType(DrawType::Triangles);
	};

	virtual bool init() { return true; };
	virtual void draw(const VertexArrayObject& vao) const = 0;
	virtual void clear() const = 0;

	virtual void render() = 0;
	virtual void renderScene(Scene* scene) = 0;

	void SetDrawType(DrawType drawType);

protected:
	uint8_t m_drawType = 0;
};