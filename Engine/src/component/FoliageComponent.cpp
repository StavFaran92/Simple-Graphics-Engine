#include "component/FoliageComponent.h"

#include "component/Transformation.h"
#include "systems/FoliageSystem.h"
#include "core/Random.h"

#include <GL/glew.h> // TODO Remove

void FoliageComponent::build()
{
	//if (transformations.empty()) return;

	//matrices.clear();
	//matrices.reserve(transformations.size());

	//for (int i = 0; i < transformations.size(); i++)
	//{
	//	matrices.push_back(transformations[i]->getWorldTransformation());
	//}

    int width = m_foliageSpreadMap->getWidth();
    int height = m_foliageSpreadMap->getHeight();

    std::vector<glm::vec4> foliageLocations;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            glm::vec4 position = glm::vec4(
                static_cast<float>(x),
                0.0f, // flat on ground (you can add noise/height here)
                static_cast<float>(y),
                1.0f
            );

            foliageLocations.push_back(position);
        }
    }

	Engine::get()->getSubSystem<FoliageSystem>()->setMeshLocations(foliageLocations);
}