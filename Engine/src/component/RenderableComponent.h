#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI RenderableComponent : public Component
{
RenderableComponent() = default;

static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

template <class Archive>
void serialize(Archive& archive) {
SERIALIZED_MEMBER(renderTechnique);
}

enum class RenderTechnique : int
{
Forward,
Deferred
};

RenderTechnique renderTechnique = RenderTechnique::Deferred;
};

REGISTER_COMPONENT(RenderableComponent)
