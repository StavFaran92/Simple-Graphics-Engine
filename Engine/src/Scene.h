#pragma once

#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>

class Model;
class Skybox;
class Context;
class Object3D;
class PointLight;
class DirectionalLight;
class Renderer;

class Scene
{

public:
	// -------------------- Methods -------------------- //
	
	bool addModel(std::shared_ptr<Model> model);
	bool removeModel(uint32_t id);
	bool removeModel(std::shared_ptr<Model> model);

	bool addPointLight(std::shared_ptr<PointLight> pLight);
	bool removePointLight(std::shared_ptr<PointLight> pLight);

	bool addDirectionalLight(std::shared_ptr<DirectionalLight> dLight);
	bool removeDirectionalLight(std::shared_ptr<DirectionalLight> dLight);

	bool addObject(Object3D* object);
	void removeObject(std::shared_ptr<Object3D> object);

	void setSkybox(std::shared_ptr<Skybox> skybox);
	void removeSkybox();

	std::shared_ptr<Renderer> getRenderer() const;
	std::shared_ptr<Renderer> getSkyboxRenderer();

	std::shared_ptr<Skybox> getSkybox();

private:
	// -------------------- Methods -------------------- //
	friend class Context;
	void update(float deltaTime);
	inline void SetID(uint32_t id) { m_id = id; }
	void draw();

	void init();
	void clear();
	void close();

private:
	// -------------------- Attributes -------------------- //
	std::unordered_map<uint32_t, std::shared_ptr<Model>> m_models;
	uint32_t m_modelCounter = 0;
	std::unordered_map<uint32_t, std::shared_ptr<PointLight>> m_pointLights;
	uint32_t m_pointLightCounter = 0;
	std::unordered_map<uint32_t, std::shared_ptr<DirectionalLight>> m_directionalLights;
	uint32_t m_directionalLightCounter = 0;

	std::shared_ptr<Renderer> m_renderer = nullptr;
	std::shared_ptr<Renderer> m_skyboxRenderer = nullptr;

	uint32_t m_id = 0;


	std::shared_ptr<Skybox> m_skybox = nullptr;

};
