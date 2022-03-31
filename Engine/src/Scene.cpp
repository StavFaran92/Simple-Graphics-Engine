#include "Scene.h"

#include "Skybox.h"
#include "Object3D.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Engine.h"
#include "ICamera.h"
#include "Model.h"
#include "ObjectSelection.h"
#include "SkyboxRenderer.h"
#include "ScreenBufferProjector.h"

void Scene::init()
{
	m_renderer = std::make_shared<Renderer>();
	m_skyboxRenderer = std::make_shared<SkyboxRenderer>(*m_renderer.get());

	m_objectSelection = std::make_shared<ObjectSelection>();
	m_objectSelection->init();

	m_screenBufferProjector = std::make_shared<ScreenBufferProjector>();
	if (!m_screenBufferProjector->init())
	{
		logError("Screen buffer projector failed to init!");
	}

	auto light = new DirectionalLight();
	addDirectionalLight(light);
}

void Scene::update(float deltaTime)
{
	
	m_renderer->Clear();

	m_renderer->GetCamera()->update(deltaTime);

	//Update models
	for (auto model = m_models.begin(); model != m_models.end(); ++model)
	{
		model->second->update(deltaTime);
	}

	if (m_skybox)
		m_skybox->update(deltaTime);
}

void Scene::draw(float deltaTime)
{
	if (m_isPostProcessEnabled && m_screenBufferProjector)
	{
		m_screenBufferProjector->RedirectToFrameBuffer();
	}

	// Draw Engine models
	for (auto model = m_models.begin(); model != m_models.end(); ++model)
	{
		auto shader = model->second->GetShader();
		shader->use();

		if (shader->IsLightsEnabled())
		{
			// Use all directional lights
			{
				int i = 0;
				for (auto it = m_directionalLights.begin(); it != m_directionalLights.end(); ++it, ++i) {
					it->second->useLight(shader, i);
				}
				shader->SetInt("dirLightCount", m_directionalLights.size());
			}

			// Use all point lights
			{
				int i = 0;
				for (auto it = m_pointLights.begin(); it != m_pointLights.end(); ++i, ++it) {
					it->second->useLight(shader, i);
				}
				shader->SetInt("pointLightCount", m_pointLights.size());
			}
		}

		glStencilFunc(GL_ALWAYS, model->second->getID(), 0xff);

		// Draw model
		model->second->Draw(m_renderer, shader);

		shader->release();
	}

	// Draw Application models
	while(!m_drawQueue.empty())
	{
		auto model = m_drawQueue.front();
		m_drawQueue.pop_front();

		model->update(deltaTime);

		auto shader = model->GetShader();
		shader->use();

		if (shader->IsLightsEnabled())
		{
			// Use all directional lights
			{
				int i = 0;
				for (auto it = m_directionalLights.begin(); it != m_directionalLights.end(); ++it, ++i) {
					it->second->useLight(shader, i);
				}
				shader->SetInt("dirLightCount", m_directionalLights.size());
			}

			// Use all point lights
			{
				int i = 0;
				for (auto it = m_pointLights.begin(); it != m_pointLights.end(); ++i, ++it) {
					it->second->useLight(shader, i);
				}
				shader->SetInt("pointLightCount", m_pointLights.size());
			}
		}

		glStencilFunc(GL_ALWAYS, model->getID(), 0xff);

		// Draw model
		model->Draw(m_renderer, shader);

		shader->release();
	}

	if (m_skybox)
	{
		m_skybox->use();
		m_skybox->Draw(m_skyboxRenderer);
	}

	if (m_isPostProcessEnabled && m_screenBufferProjector)
	{
		m_screenBufferProjector->RedirectToDefault();
	}
}

void Scene::clear()
{
	m_models.clear();
	m_pointLights.clear();
	m_directionalLights.clear();
}

bool Scene::addModel(Model* model)
{
	m_modelCounter++;
	model->setID(m_modelCounter);
	m_models.emplace(m_modelCounter, std::shared_ptr<Model>( model));

	logInfo("Model {} Added successfully.", std::to_string(m_modelCounter));
	return true;
}

bool Scene::addPointLight(PointLight* pLight)
{
	m_pointLightCounter++;
	pLight->setID(m_pointLightCounter);
	m_pointLights.emplace(m_pointLightCounter, std::shared_ptr<PointLight>(pLight));

	logInfo("PointLight {} Added successfully.", std::to_string(m_pointLightCounter));
	return true;
}

bool Scene::addDirectionalLight(DirectionalLight* dLight)
{
	m_directionalLightCounter++;
	dLight->setID(m_directionalLightCounter);
	m_directionalLights.emplace(m_directionalLightCounter, std::shared_ptr<DirectionalLight>(dLight));

	logInfo("DirectionalLight {} Added successfully.", std::to_string(m_directionalLightCounter));
	return true;
}

bool Scene::removeModel(uint32_t id)
{
	auto iter = m_models.find(id);
	if (iter == m_models.end())
	{
		logError("Could not locate Model {}", id);
		return false;
	}
	m_models.erase(iter);

	logInfo("Model {} Erased successfully.", std::to_string(id));

	return true;
}

bool Scene::removeModel(Model* model)
{
		uint32_t id = model->getID();
		
		return removeModel(id);
}

bool Scene::removePointLight(PointLight* pLight)
{
	uint32_t id = pLight->getID();
	auto iter = m_pointLights.find(id);
	if (iter == m_pointLights.end())
	{
		logError("Could not locate PointLight {}", id);
		return false;
	}
	m_pointLights.erase(iter);

	logInfo("PointLight {} Erased successfully.", std::to_string(id));

	return true;
}

bool Scene::removeDirectionalLight(DirectionalLight* dLight)
{
	uint32_t id = dLight->getID();
	auto iter = m_directionalLights.find(id);
	if (iter == m_directionalLights.end())
	{
		logError("Could not locate DirectionalLight {}", id);
		return false;
	}
	m_directionalLights.erase(iter);

	logInfo("DirectionalLight {} Erased successfully.", std::to_string(id));

	return true;
}


void Scene::setSkybox(Skybox* skybox)
{
	m_skybox = std::shared_ptr<Skybox>(skybox);
}

void Scene::removeSkybox()
{
	m_skybox = nullptr;
}

std::shared_ptr<Renderer> Scene::getRenderer() const
{
	return m_renderer;
}

std::shared_ptr<Renderer> Scene::getSkyboxRenderer()
{
	return m_skyboxRenderer;
}

void Scene::close()
{
	clear();
}

void Scene::setPostProcess(bool value)
{
	m_isPostProcessEnabled = value;
}

std::shared_ptr<ObjectSelection> Scene::GetObjectSelection() const
{
	return m_objectSelection;
}

void Scene::draw(Model* model)
{
	m_drawQueue.push_back(model);
}

bool Scene::setPostProcessShader(Shader* shader)
{
	if (m_screenBufferProjector)
	{
		m_screenBufferProjector->setPostProcessShader(std::shared_ptr<Shader>(shader));
		return true;
	}
	return false;
}

std::shared_ptr<Skybox> Scene::getSkybox()
{
	return m_skybox;
}