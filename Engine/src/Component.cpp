#include "Component.h"

#include "Engine.h"
#include "Context.h"
#include "Material.h"
#include "Transformation.h"
#include <GL/glew.h>

MaterialComponent::MaterialComponent()
{
	auto mat = std::make_shared<Material>(*Engine::get()->getDefaultMaterial().get());
	materials[0] = mat;
}

InstanceBatch::InstanceBatch()
{
	glGenBuffers(1, &m_id);
}

InstanceBatch::InstanceBatch(const std::vector<std::shared_ptr<Transformation>>& transformations, Resource<Mesh> mesh)
	: mesh(mesh), transformations(transformations)
{
	glGenBuffers(1, &m_id);

	build();
}

void InstanceBatch::addTransformation(const std::shared_ptr<Transformation>& transformation)
{
	transformations.push_back(transformation);

	build();
}

std::vector<std::shared_ptr<Transformation>>& InstanceBatch::getTransformations()
{
	return transformations;
}

const std::vector<glm::mat4> InstanceBatch::getMatrices() const
{
	std::vector<glm::mat4> matrices;
	matrices.reserve(transformations.size());

	for (int i = 0; i < transformations.size(); i++)
	{
		matrices.push_back(transformations[i]->getWorldTransformation());
	}

	return matrices;
}

void InstanceBatch::build()
{
	if (mesh.isEmpty() || transformations.empty()) return;

	auto matrices = getMatrices();

	mesh.get()->getVAO()->Bind();
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
	glBufferData(GL_ARRAY_BUFFER, getCount() * sizeof(glm::mat4), matrices.data(), GL_STATIC_DRAW);

	//VertexLayout layout;
	//layout.attribs = {
	//	LayoutAttribute::InstanceModel_0,
	//	LayoutAttribute::InstanceModel_1,
	//	LayoutAttribute::InstanceModel_2,
	//	LayoutAttribute::InstanceModel_3,
	//};

	//mesh.get()->setVertexLayout(layout);

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(9);
	glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);
	glVertexAttribDivisor(9, 1);
}

void RigidBodyComponent::addForce(glm::vec3 force)
{
	m_force += force;
	isChanged = true;
}

void RigidBodyComponent::setForce(glm::vec3 force)
{
	m_force = glm::vec3(0);
	addForce(force);
}

void RigidBodyComponent::move(glm::vec3 position)
{
	m_targetPisition = position;
	isChanged = true;
}

glm::mat4 CameraComponent::getProjection() const
{
	if (type == CamType::PERSPECTIVE) return glm::perspective(fovy, aspect, znear, zfar);
	if (type == CamType::ORTHOGRAPHIC) return glm::ortho(-100, 100, -100, 100);
}

ShaderComponent::ShaderComponent()
{
	renderViewProjection = std::make_shared<RenderView>(RenderView::Viewport{ 0, 0, 1920, 1080 }, Entity::EmptyEntity);
};

ShaderComponent::ShaderComponent(Shader* vertexShader, Shader* fragmentShader)
	: m_vertexShader(vertexShader), m_fragmentShader(fragmentShader)
{
	renderViewProjection = std::make_shared<RenderView>(RenderView::Viewport{ 0, 0, 1920, 1080 }, Entity::EmptyEntity);
};

void ShaderComponent::setProjectionTexture(Resource<Texture> texture)
{
	renderViewProjection->bind();
	renderViewProjection->setTexture(texture);
	projectionTexture = texture;
	
};
