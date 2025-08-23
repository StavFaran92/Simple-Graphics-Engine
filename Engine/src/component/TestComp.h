#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct TestComp : public Component
{
        TestComp() = default;

        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

        template <class Archive>
        void serialize(Archive& archive) {
                SERIALIZED_MEMBER(test);
                SERIALIZED_MEMBER_OPTIONAL(test2, 0);
        }

        int test = 0;
        int test2 = 0;
};

REGISTER_COMPONENT(TestComp)
