#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include <glm/glm.hpp>

struct EngineAPI PlayerController : public Component
{
        PlayerController() = default;

        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

        template <class Archive>
        void serialize(Archive& archive) {
        }

        void move(glm::vec3 disp)
        {
                m_disp = disp;
        }

        void reset()
        {
                m_disp = glm::vec3(0.f);
        }

    glm::vec3 m_disp{};
    int controllerIndex = 0;
};

REGISTER_COMPONENT(PlayerController)
