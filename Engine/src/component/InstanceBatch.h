#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "component/Transformation.h"
#include "geometry/Mesh.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

struct EngineAPI InstanceBatch : public Component
{
public:
        InstanceBatch();

        InstanceBatch(const std::vector<std::shared_ptr<Transformation>>& transformations, Resource<Mesh> mesh);

        void addTransformation(const std::shared_ptr<Transformation>& transformation);

        const std::vector<glm::mat4> getMatrices() const;
        std::vector<std::shared_ptr<Transformation>>& getTransformations();

        size_t getCount() const
        {
                return transformations.size();
        }

        void build();

        std::vector<std::shared_ptr<Transformation>> transformations;
        Resource<Mesh> mesh;
        unsigned int m_id = 0;
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
private:

private:

};

REGISTER_COMPONENT(InstanceBatch)
