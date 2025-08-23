#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "render/Shader.h"

struct VolumeComponent : public Component
{
        VolumeComponent() = default;

        template <class Archive>
        void serialize(Archive& archive) {
        }

        Resource<Shader> shader;
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};

REGISTER_COMPONENT(VolumeComponent)
