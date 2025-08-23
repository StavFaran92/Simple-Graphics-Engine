#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "texture/Texture.h"
#include <glm/glm.hpp>

struct EngineAPI ImageComponent : public Component
{
        ImageComponent() = default;

        ImageComponent(Resource<Texture> image) : image(image) {}

        template <class Archive>
        void serialize(Archive& archive) {
                SERIALIZED_MEMBER(image);
                SERIALIZED_MEMBER(size);
                SERIALIZED_MEMBER(position);
                SERIALIZED_MEMBER(rotate);
        }

        glm::vec2 size;
        glm::vec2 position;
        float rotate = 0;

        Resource<Texture> image;
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};

REGISTER_COMPONENT(ImageComponent)
