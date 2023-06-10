#pragma once

// Includes
//STL
#include <memory>

#include "glm/glm.hpp"
#include "Core.h"

class Shader;
class VertexArrayObject;
class Model;
class ICamera;
class Transformation;
class Mesh;
class Entity;

class EngineAPI IRenderer {
public:
	enum class DrawType {
		Lines,
		Triangles
	};
public:
	IRenderer() {
		SetDrawType(DrawType::Triangles);
	};

	//virtual void render(Model* model, Shader* shader = nullptr) = 0;
	virtual void draw(const VertexArrayObject& vao, Shader& shader) const = 0;
	virtual void clear() const = 0;

	virtual glm::mat4 getProjection() const = 0;
	virtual std::shared_ptr<ICamera> getCamera() const = 0;
	virtual void render(Entity* entity, Mesh* mesh, Transformation* transform, Shader* shader = nullptr) = 0;

	void SetDrawType(DrawType drawType);

protected:
	uint8_t m_drawType = 0;
};