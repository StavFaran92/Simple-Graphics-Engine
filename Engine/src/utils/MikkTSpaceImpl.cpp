#include "utils/MikkTSpaceImpl.h"

#include "geometry/Mesh.h"

struct MikkVertexRef {
    glm::vec3& position;
    glm::vec3& normal;
    glm::vec2& texCoord;
    glm::vec4& tangent;
};

MikkVertexRef getVertexRef(MeshData* mesh, int face, int vert) {
    int idx = mesh->indices[face * 3 + vert];

    if (mesh->getType() == MeshType::SkinnedMesh) {
        auto& v = mesh->skinnedVertices[idx];
        return { v.position, v.normal, v.texCoord, v.tangent };
    }

    auto& v = mesh->staticVertices[idx];
    return { v.position, v.normal, v.texCoord, v.tangent };
}

// --- Callbacks ---

int getNumFaces(const SMikkTSpaceContext* context) {
    auto* userData = (MikkMeshContext*)context->m_pUserData;
    return static_cast<int>(userData->mesh->indices.size()) / 3;
}

int getNumVertsOfFace(const SMikkTSpaceContext*, int) {
    return 3; // Always triangles
}

void getPosition(const SMikkTSpaceContext* context, float outPos[3], int face, int vert) {
    auto* userData = (MikkMeshContext*)context->m_pUserData;
    auto ref = getVertexRef(userData->mesh, face, vert);
    memcpy(outPos, &ref.position, sizeof(float) * 3);
}

void getNormal(const SMikkTSpaceContext* context, float outNormal[3], int face, int vert) {
    auto* userData = (MikkMeshContext*)context->m_pUserData;
    auto ref = getVertexRef(userData->mesh, face, vert);
    memcpy(outNormal, &ref.normal, sizeof(float) * 3);
}

void getTexCoord(const SMikkTSpaceContext* context, float outUV[2], int face, int vert) {
    auto* userData = (MikkMeshContext*)context->m_pUserData;
    auto ref = getVertexRef(userData->mesh, face, vert);
    memcpy(outUV, &ref.texCoord, sizeof(float) * 2);
}

void setTSpaceBasic(const SMikkTSpaceContext* context, const float tangent[3], float sign, int face, int vert) {
    auto* userData = (MikkMeshContext*)context->m_pUserData;
    auto ref = getVertexRef(userData->mesh, face, vert);
    ref.tangent = glm::vec4(tangent[0], tangent[1], tangent[2], sign);
}