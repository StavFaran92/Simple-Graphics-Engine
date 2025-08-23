#include "component/Component.h"
#include "component/TagComponent.h"
#include "component/SkyboxComponent.h"
#include "component/RenderableComponent.h"
#include "component/NativeScriptComponent.h"
#include "component/PhysicsComponent.h"
#include "component/CameraComponent.h"
#include "component/MeshComponent.h"
#include "component/MaterialComponent.h"
#include "component/ObjectComponent.h"
#include "component/ShaderComponent.h"
#include "component/InstanceBatch.h"
#include "component/ImageComponent.h"
#include "component/PlayerController.h"
#include "component/VolumeComponent.h"
#include "component/CharacterController.h"
#include "component/TestComp.h"

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

template<typename T>
static void attachSimple(std::shared_ptr<Component> c, Entity entityHandler)
{
    if (auto tc = std::dynamic_pointer_cast<T>(c))
    {
		entityHandler.addComponent<T>(*tc);
    }
}

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

void RenderableComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
    (void)scene;
    attachSimple<RenderableComponent>(c, entityHandler);
}

void NativeScriptComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto nc = std::dynamic_pointer_cast<NativeScriptComponent>(c))
	{
		auto& nsc = entityHandler.addComponent<NativeScriptComponent>(*nc);
		nsc.entity.setRegistry(&scene.getRegistry());
		if (nsc.script)
		{
			nsc.script->entity.setRegistry(&scene.getRegistry());
		}
	}
}

void PhysicsComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
    (void)scene;
    attachSimple<PhysicsComponent>(c, entityHandler);
}

void CameraComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
    (void)scene;
    attachSimple<CameraComponent>(c, entityHandler);
}

void MeshComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
    (void)scene;
    attachSimple<MeshComponent>(c, entityHandler);
}

void MaterialComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
    (void)scene;
    attachSimple<MaterialComponent>(c, entityHandler);
}

void ObjectComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto oc = std::dynamic_pointer_cast<ObjectComponent>(c))
	{
		auto& obj = entityHandler.addComponent<ObjectComponent>(*oc);
		obj.e.setRegistry(&scene.getRegistry());
	}
}

void ShaderComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	(void)scene;
	if (auto sc = std::dynamic_pointer_cast<ShaderComponent>(c))
	{
		auto& shader = entityHandler.addComponent<ShaderComponent>(*sc);
		shader.update();
	}
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

void VolumeComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
        (void)scene;
        attachSimple<VolumeComponent>(c, entityHandler);
}

void CharacterController::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
        (void)scene;
        attachSimple<CharacterController>(c, entityHandler);
}


MaterialComponent::MaterialComponent()
{
	auto mat = Engine::get()->getDefaultMaterial()->clone();

	//auto mat = std::make_shared<Material>(*Engine::get()->getDefaultMaterial().get());
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

void PhysicsComponent::addForce(glm::vec3 force)
{
	m_force += force;
	isChanged = true;
}

void PhysicsComponent::setForce(glm::vec3 force)
{
	m_force = glm::vec3(0);
	addForce(force);
}

void PhysicsComponent::move(glm::vec3 position)
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
	renderViewProjection = std::make_shared<RenderView>(Viewport{ 0, 0, 1920, 1080 }, Entity::EmptyEntity);
};

void ShaderComponent::setProjectionTexture(Resource<Texture> texture)
{
	renderViewProjection->bind();
	renderViewProjection->setTexture(texture);
	projectionTexture = texture;
	
}

#include <regex>

void ShaderComponent::parseUniforms(const std::string& sourceCode)
{
	m_uniformProperties.clear();
	customTextures.clear();

	std::regex uniformRegex(R"(uniform\s+(\w+)\s+(\w+)\s*;)");
	std::smatch match;
	std::string::const_iterator searchStart(sourceCode.cbegin());

	auto& uniformProperties = m_uniformProperties;

	while (std::regex_search(searchStart, sourceCode.cend(), match, uniformRegex)) {
		std::string type = match[1].str();
		std::string name = match[2].str();

		if (type == "float") {
			uniformProperties[name] = 0.0f;
		}
		else if (type == "vec2") {
			uniformProperties[name] = glm::vec2(0.0f);
		}
		else if (type == "vec3") {
			uniformProperties[name] = glm::vec3(0.0f);
		}
		else if (type == "vec4") {
			uniformProperties[name] = glm::vec4(0.0f);
		}
		else if (type == "int") {
			uniformProperties[name] = 0;
		}
		else if (type == "uint") {
			uniformProperties[name] = 0u;
		}
		else if (type == "mat3") {
			uniformProperties[name] = glm::mat3(1.0f);
		}
		else if (type == "mat4") {
			uniformProperties[name] = glm::mat4(1.0f);
		}
		else if (type == "sampler2D") {
			customTextures[name] = Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::WHITE_1X1);
		}

		searchStart = match.suffix().first;
	}
}

void ShaderComponent::setShader(Resource<Shader> shader)
{
	m_customShader = shader;

	shaderOverride = shader->getShaderOverride();

	const std::string& sourceCode = shader->getSourceCode();

	parseUniforms(sourceCode);

	isValid = true;
}

void ShaderComponent::update()
{
	auto oldUniforms = m_uniformProperties;
	auto oldTextures = customTextures;

	parseUniforms(m_customShader->getSourceCode());


	auto& newTextures = customTextures;
	for (const auto [name, texture] : oldTextures)
	{
		auto iter = newTextures.find(name);
		if (iter != newTextures.end())
		{
			iter->second = texture;
		}
	}

	auto& newUniforms = m_uniformProperties;
	for (const auto [name, value] : oldUniforms)
	{
		auto iter = newUniforms.find(name);
		if (iter != newUniforms.end())
		{
			iter->second = value;
		}
	}

	for (const auto& [name, value] : m_uniformProperties)
	{
		m_customShader.get()->setUniformValue(name, value);
	}

	if (!projectionTexture.isEmpty())
	{
		setProjectionTexture(projectionTexture);
	}
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