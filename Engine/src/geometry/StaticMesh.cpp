#include "geometry/StaticMesh.h"

StaticMesh::StaticMesh()
{
}

std::vector<glm::vec3> StaticMesh::getPositions() const
{
    return std::vector<glm::vec3>();
}

std::vector<glm::vec3> StaticMesh::getNormals() const
{
    return std::vector<glm::vec3>();
}

MeshType StaticMesh::getMeshType() const
{
    return MeshType::StaticMesh;
}
