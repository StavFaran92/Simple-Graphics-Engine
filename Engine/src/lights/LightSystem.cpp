#include "LightSystem.h"

#include "UniformBufferObject.h"
#include "Scene.h"
#include "Context.h"
#include "IRenderer.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Transformation.h"
#include "Graphics.h"

#define NR_POINT_LIGHTS 32
#define NR_DIR_LIGHTS 8

struct PointLightUBORep
{
	glm::vec4 position;
	glm::vec4 color;
};

struct DirLightUBORep
{
	glm::vec4 direction;
	glm::vec4 color;
};

LightSystem::LightSystem(Context* context, Scene* scene)
	: m_context(context), m_scene(scene)
{
}

bool LightSystem::init()
{
	/*
		The Buffer block is:

		The following specified the std140 used alignment
		N = 4 bytes in memory

		----------------------------
		layout (std140) uniform Lights					// base alignment									// aligned offset
		{
			int pointLightCount;						// N												// 0
			int dirLightCount;							// N												// N
			PointLight pointLights[NR_POINT_LIGHTS];	// sizeof(PointLightUBORep) * NR_POINT_LIGHTS		// 4N
			DirLight dirLight[NR_DIR_LIGHT];			// sizeof(DirLightUBORep) * NR_POINT_LIGHTS			// 4N + sizeof(PointLightUBORep) * NR_POINT_LIGHTS

		};
		-----------------------------

		Total struct size = sizeof(DirLightUBORep) * NR_DIR_LIGHTS + 4N + sizeof(PointLightUBORep) * NR_POINT_LIGHTS
		size = NR_DIR_LIGHTS * sizeof(DirLightUBORep) + NR_POINT_LIGHTS * sizeof(PointLightUBORep) + 2 * sizeof(int) + 8

		e.g. 
		NR_DIR_LIGHTS = 8
		NR_POINT_LIGHTS = 8
		sizeof(DirLightUBORep) = 4N * 2
		sizeof(DirLightUBORep) = 4N * 2
		sizeof(int) = N

		Total size = 8 * 4N * 2 + 8 * 4N * 2 + 2 * 4 + 8 = 64N + 64N + 8 + 8 = 128N + 16 = 528 bytes

		** The 8 at the end is used for padding **
 
	*/

	unsigned int bufferSize = NR_DIR_LIGHTS * sizeof(DirLightUBORep) + NR_POINT_LIGHTS * sizeof(PointLightUBORep) + 2 * sizeof(int) + 8;
	m_uboLights = std::make_shared<UniformBufferObject>(bufferSize);
	m_uboLights->attachToBindPoint(1);

	m_scene->addRenderCallback(Scene::RenderPhase::PRE_RENDER_BEGIN, [=]() {

		setLightsInUBO();
		});

    return true;
}

void LightSystem::setLightsInUBO()
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	m_uboLights->bind();

	{
		// Use all point lights
		auto view = m_scene->getRegistry().getRegistry().view<PointLight, Transformation>();

		int i = 0;
		for (auto it = view.begin(); it != view.end(); ++it, ++i)
		{
			auto& pLight = view.get<PointLight>(*it);
			auto& transform = view.get<Transformation>(*it);

			PointLightUBORep pointLightUBO;
			auto pos = /**params->view **/ glm::vec4(transform.getLocalPosition(), 1.0f);
			pointLightUBO.position = { glm::vec3(pos.x,pos.y, pos.z), 1.f};
			pointLightUBO.color = { pLight.getColor(), 1.f };

			m_uboLights->setData(2 * sizeof(int) + 8 + sizeof(PointLightUBORep) * i, sizeof(pointLightUBO.position), glm::value_ptr(pointLightUBO.position));
			m_uboLights->setData(2 * sizeof(int) + 8 + sizeof(PointLightUBORep) * i + sizeof(pointLightUBO.position), sizeof(pointLightUBO.color), glm::value_ptr(pointLightUBO.color));
		}
		m_uboLights->setData(0, sizeof(int), &i);
	}

	{
		// Use all Directional lights
		auto view = m_scene->getRegistry().getRegistry().view<DirectionalLight>();

		int i = 0;
		for (auto it = view.begin(); it != view.end(); ++it, ++i)
		{
			auto& dLight = view.get<DirectionalLight>(*it);
			Entity e(*it, &m_scene->getRegistry());
			auto& dir = e.getComponent<Transformation>().getLocalRotationVec3();

			DirLightUBORep dirLightUBO;
			auto dir4 = /**params->view **/ glm::vec4(dir, 1.0f);
			dirLightUBO.direction = { glm::vec3(dir4.x, dir4.y, dir4.z) , 1.f};
			dirLightUBO.color = { dLight.getColor(), 1.f };

			m_uboLights->setData(2 * sizeof(int) + 8 + sizeof(PointLightUBORep) * NR_POINT_LIGHTS + sizeof(DirLightUBORep) * i, sizeof(dirLightUBO.direction), glm::value_ptr(dirLightUBO.direction));
			m_uboLights->setData(2 * sizeof(int) + 8 + sizeof(PointLightUBORep) * NR_POINT_LIGHTS + sizeof(DirLightUBORep) * i + sizeof(dirLightUBO.direction), sizeof(dirLightUBO.color), glm::value_ptr(dirLightUBO.color));
		}

		m_uboLights->setData(sizeof(int), sizeof(int), &i);
	}

	m_uboLights->unbind();


}
