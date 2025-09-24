#include "utils/DebugHelper.h"

#include "render/VertexArrayObject.h"
#include "render/VertexLayout.h"
#include "render/Shader.h"
#include "render/Graphics.h"

#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

DebugHelper::DebugHelper()
{
	m_vao = std::make_shared<VertexArrayObject>();

	m_pointShader = Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/PointDebugShader.glsl");
	m_lineShader = Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/LineDebugShader.glsl");
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
	m_pointShader->setViewMatrix(*graphics->view);
	m_pointShader->setProjectionMatrix(*graphics->projection);

	m_vao->Bind();

	// Draw the point
	glDrawArrays(GL_POINTS, 0, 1);
}

void DebugHelper::drawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color/* = { 1,0,0 }*/)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	// Set the size of the point
	glLineWidth(10); // Size in pixels

	m_lineShader->use();
	m_lineShader->setUniformValue("color", color);
	m_lineShader->setUniformValue("p1", p1);
	m_lineShader->setUniformValue("p2", p2);
	m_lineShader->setViewMatrix(*graphics->view);
	m_lineShader->setProjectionMatrix(*graphics->projection);

	m_vao->Bind();

	// Draw the point
	glDrawArrays(GL_LINES, 0, 2);
}

void DebugHelper::drawTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
}

void DebugHelper::registerTextureForDebug(const std::string& name, ResourceWrapper<Texture> texture)
{
	m_debugTextures[name] = texture;
}

const std::map<std::string, ResourceWrapper<Texture>>& DebugHelper::getDebugTextures() const
{
	return m_debugTextures;
}
