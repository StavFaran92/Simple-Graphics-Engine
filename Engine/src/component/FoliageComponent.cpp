#include "FoliageComponent.h"

#include "Transformation.h"
#include "systems/FoliageSystem.h"
#include "Random.h"

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

    unsigned int tex = m_foliageSpreadMap->getID();

    int width = m_foliageSpreadMap->getWidth();
    int height = m_foliageSpreadMap->getHeight();
    m_foliageSpreadMap->bind();

    // Allocate buffer for 1 channel per pixel
    std::vector<GLubyte> pixels(width * height * 3);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    std::vector<glm::vec4> foliageLocations;

    const int densityMultiplier = 255;
    RandomNumberGenerator rng;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int i = (y * width + x) * 3;
            GLubyte r = pixels[i];

            if (r > 0.f) {
                float scale = r / 255.f;
                int bladeCount = density * scale * densityMultiplier;

                for (int j = 0; j < bladeCount; j++)
                {
                    float xoffset = rng.rand();
                    float yoffset = rng.rand();

                    glm::vec4 position = glm::vec4(
                        static_cast<float>(x) + xoffset,
                        0.0f, // flat on ground (you can add noise/height here)
                        static_cast<float>(y) + yoffset,
                        1.0f
                    );

                    foliageLocations.push_back(position);
                }
            }
        }
    }

	Engine::get()->getSubSystem<FoliageSystem>()->setMeshLocations(foliageLocations);
}