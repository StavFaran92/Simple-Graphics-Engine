#include "utils/MikkTSpaceImpl.h"

#include "geometry/Mesh.h"

int getNumFaces(const SMikkTSpaceContext* context) {
    auto* userData = (MikkMeshContext*)context->m_pUserData;
    return static_cast<int>(userData->mesh->m_indices.size()) / 3;
}

int getNumVertsOfFace(const SMikkTSpaceContext*, int) {
    return 3; // Always triangles
}

void getPosition(const SMikkTSpaceContext* context, float outPos[3], int face, int vert) {
    auto* userData = (MikkMeshContext*)context->m_pUserData;
    int idx = userData->mesh->m_indices[face * 3 + vert];
    const glm::vec3& pos = userData->mesh->m_positions[idx];
    memcpy(outPos, &pos, sizeof(float) * 3);
}

void getNormal(const SMikkTSpaceContext* context, float outNormal[3], int face, int vert) {
    auto* userData = (MikkMeshContext*)context->m_pUserData;
    int idx = userData->mesh->m_indices[face * 3 + vert];
    const glm::vec3& norm = userData->mesh->m_normals[idx];
    memcpy(outNormal, &norm, sizeof(float) * 3);
}

void getTexCoord(const SMikkTSpaceContext* context, float outUV[2], int face, int vert) {
    auto* userData = (MikkMeshContext*)context->m_pUserData;
    int idx = userData->mesh->indices[face * 3 + vert];
    const glm::vec2& uv = userData->mesh->staticVertices[idx].texCoord;
    memcpy(outUV, &uv, sizeof(float) * 2);
}

void setTSpaceBasic(const SMikkTSpaceContext* context, const float tangent[3], float sign, int face, int vert) {
    auto* userData = (MikkMeshContext*)context->m_pUserData;
    int idx = userData->mesh->m_indices[face * 3 + vert];
    userData->mesh->m_tangents[idx] = glm::vec4(tangent[0], tangent[1], tangent[2], sign);
}