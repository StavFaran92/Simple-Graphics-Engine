#include "utils/DebugHelper.h"

#include "render/VertexArrayObject.h"
#include "render/VertexLayout.h"
#include "render/Shader.h"
#include "render/Graphics.h"
#include "geometry/AABB.h"

#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

DebugHelper::DebugHelper()
{
	m_vao = std::make_shared<VertexArrayObject>();

	m_pointShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PointDebugShader.glsl");
	m_lineShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/LineDebugShader.glsl");
}


void DebugHelper::drawPoint(const glm::vec3& pos, const glm::vec3& color/* = { 1,0,0 }*/, float size/* = 1*/)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	// Set the size of the point
	glPointSize(size); // Size in pixels

	auto model = glm::translate(glm::mat4(1.0f), pos);

	m_pointShader->use();
	m_pointShader->setUniformValue("color", color);
	m_pointShader->setModelMatrix(model);
	m_pointShader->setViewMatrix(graphics->view);
	m_pointShader->setProjectionMatrix(graphics->projection);

	m_vao->Bind();

	// Draw the point
	glDrawArrays(GL_POINTS, 0, 1);
}

void DebugHelper::drawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color/* = { 1,1,1 }*/, float thickness /*= 10*/)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	// Set the size of the point
	glLineWidth(thickness); // Size in pixels

	m_lineShader->use();
	m_lineShader->setUniformValue("color", color);
	m_lineShader->setUniformValue("p1", p1);
	m_lineShader->setUniformValue("p2", p2);
	m_lineShader->setViewMatrix(graphics->view);
	m_lineShader->setProjectionMatrix(graphics->projection);

	m_vao->Bind();

	// Draw the point
	glDrawArrays(GL_LINES, 0, 2);

	// Revert to previous shader
	if(graphics->shader)
		graphics->shader->use();
}

void DebugHelper::drawTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
}

void DebugHelper::drawAABB(const AABB& aabb, const glm::vec3& color, float thickness)
{
	glm::vec3 c = aabb.center();
	glm::vec3 e = aabb.extents();

	// Compute 8 corners from center and extents
	glm::vec3 corners[8] = {
		c + glm::vec3(-e.x, -e.y, -e.z),
		c + glm::vec3(e.x, -e.y, -e.z),
		c + glm::vec3(e.x,  e.y, -e.z),
		c + glm::vec3(-e.x,  e.y, -e.z),
		c + glm::vec3(-e.x, -e.y,  e.z),
		c + glm::vec3(e.x, -e.y,  e.z),
		c + glm::vec3(e.x,  e.y,  e.z),
		c + glm::vec3(-e.x,  e.y,  e.z)
	};

	auto& dbg = DebugHelper::getInstance();

	// Bottom face
	dbg.drawLine(corners[0], corners[1], color, thickness);
	dbg.drawLine(corners[1], corners[2], color, thickness);
	dbg.drawLine(corners[2], corners[3], color, thickness);
	dbg.drawLine(corners[3], corners[0], color, thickness);

	// Top face
	dbg.drawLine(corners[4], corners[5], color, thickness);
	dbg.drawLine(corners[5], corners[6], color, thickness);
	dbg.drawLine(corners[6], corners[7], color, thickness);
	dbg.drawLine(corners[7], corners[4], color, thickness);

	// Vertical edges
	dbg.drawLine(corners[0], corners[4], color, thickness);
	dbg.drawLine(corners[1], corners[5], color, thickness);
	dbg.drawLine(corners[2], corners[6], color, thickness);
	dbg.drawLine(corners[3], corners[7], color, thickness);
}

void DebugHelper::registerTextureForDebug(const std::string& name, ResourceWrapper<Texture> texture)
{
	m_debugTextures[name] = texture;
}

const std::map<std::string, ResourceWrapper<Texture>>& DebugHelper::getDebugTextures() const
{
	return m_debugTextures;
}
