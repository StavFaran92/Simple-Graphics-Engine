#include "component/Component.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "render/Material.h"
#include "component/Transformation.h"
#include "systems/CommonTextures.h"
#include "utils/EquirectangularToCubemapConverter.h"
#include "runtime/Scene.h"
#include "render/IBL.h"
#include <GL/glew.h>
#include "render/VertexArrayObject.h"
#include "component/ComponentSerializer.h"

void TagComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
    (void)scene;
    attachSimple<TagComponent>(c, entityHandler);
}

void SkyboxComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto sc = std::dynamic_pointer_cast<SkyboxComponent>(c))
	{
		auto& skyboxComponent = entityHandler.addComponent<SkyboxComponent>(*sc);
		skyboxComponent.build();
	}
}

void TestComp::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	(void)scene;
	attachSimple<TestComp>(c, entityHandler);
}

void InstanceBatch::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
        (void)scene;
        attachSimple<InstanceBatch>(c, entityHandler);
}

void ImageComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
        (void)scene;
        attachSimple<ImageComponent>(c, entityHandler);
}

void PlayerController::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
        (void)scene;
        attachSimple<PlayerController>(c, entityHandler);
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

SkyboxComponent::SkyboxComponent(Resource<Texture> skyboxImage)
{
	setSkybox(skyboxImage);
}

void SkyboxComponent::setSkybox(Resource<Texture> image)
{
	originalImage = image;
}

void SkyboxComponent::build()
{

	// TODO check if orig image is cube and support cubemap load

	cubemap = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(originalImage);

	auto scene = Engine::get()->getContext()->getActiveScene().get();
	auto irradianceMap = IBL::generateIrradianceMap(cubemap, scene);
	auto prefilterEnvMap = IBL::generatePrefilterEnvMap(cubemap, scene);

	scene->setIBLData(irradianceMap, prefilterEnvMap);
}