#include "component/Component.h"

#include "component/Transformation.h"
#include "runtime/Scene.h"
#include <GL/glew.h>
#include "render/VertexArrayObject.h"
#include "core/Trace.h"
#include "core/Engine.h"
#include "runtime/Context.h"

std::string InstanceBatch::getName()
{
	return "InstanceBatch";
}

InstanceBatch::InstanceBatch()
{
	glGenBuffers(1, &m_id);
}

InstanceBatch::InstanceBatch(const std::vector<std::shared_ptr<Transformation>>& transformations, ResourceRef<Mesh> mesh)
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
	//if (mesh.isEmpty() || transformations.empty()) return;

	//auto matrices = getMatrices();

	//mesh.get()->getVAO()->Bind();
	//glBindBuffer(GL_ARRAY_BUFFER, m_id);
	//glBufferData(GL_ARRAY_BUFFER, getCount() * sizeof(glm::mat4), matrices.data(), GL_STATIC_DRAW);

	////VertexLayout layout;
	////layout.attribs = {
	////	LayoutAttribute::InstanceModel_0,
	////	LayoutAttribute::InstanceModel_1,
	////	LayoutAttribute::InstanceModel_2,
	////	LayoutAttribute::InstanceModel_3,
	////};

	////mesh.get()->setVertexLayout(layout);

	//glEnableVertexAttribArray(6);
	//glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	//glEnableVertexAttribArray(7);
	//glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	//glEnableVertexAttribArray(8);
	//glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	//glEnableVertexAttribArray(9);
	//glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	//glVertexAttribDivisor(6, 1);
	//glVertexAttribDivisor(7, 1);
	//glVertexAttribDivisor(8, 1);
	//glVertexAttribDivisor(9, 1);
}

//void Component::registerSceneDependency(SceneResourceRef& scene) const
//{
//	auto assetDeps = gatherDependenciesInternal();
//	for (auto& asset : assetDeps)
//	{
//		asset.registerOnChangedListener([scene]()mutable {
//			scene->makeDirty();
//		});
//		Trace::addSceneAssetMonitor(scene, asset.get()->getUID());
//	}
//}
//
//void Component::removeSceneDependency(SceneResourceRef& scene) const
//{
//	auto assetDeps = gatherDependenciesInternal();
//	for (auto& observer : assetDeps)
//	{
//		observer.removeOnChangedListener();
//		Trace::removeSceneAssetMonitor(scene, observer.get()->getUID());
//	}
//}

std::vector<AssetRef<Asset>> Component::gatherDependencies() const
{
	std::vector<AssetRef<Asset>> result;
	auto assetDependencies = gatherDependenciesInternal();
	for (auto asset : assetDependencies)
	{
		result.push_back(asset);
	}
	return result;
}

void Component::activate()
{
	isActive = true;
}

void Component::deactivate()
{
	isActive = false;
}

std::vector<AssetRef<Asset>> Component::gatherDependenciesInternal() const
{
	return {};
}
