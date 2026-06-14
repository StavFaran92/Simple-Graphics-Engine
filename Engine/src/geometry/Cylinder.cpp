#include "geometry/Cylinder.h"
#include "core/ApplicationConstants.h"
#include "geometry/Mesh.h"
#include "geometry/MeshBuilder.h"
#include "core/Engine.h"
#include "geometry/ModelImporter.h"
#include "core/Factory.h"
#include "memory/Assets.h"

std::shared_ptr<Mesh> Cylinder::createMesh(float height, float radius, int sectors)
{
    MeshBuilder builder(MeshType::StaticMesh);

    float sectorStep = 2.0f * Constants::PI / sectors;
    float halfHeight = height * 0.5f;

    // Side vertices: (sectors+1) pairs, top at i*2, bottom at i*2+1
    for (int i = 0; i <= sectors; ++i)
    {
        float angle = i * sectorStep;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);

        StaticVertex top{};
        top.position = { x, halfHeight, z };
        top.normal = glm::normalize(glm::vec3(x, 0.0f, z));
        top.texCoord = { (float)i / sectors, 1.0f };
        builder.addVertex(top);

        StaticVertex bottom{};
        bottom.position = { x, -halfHeight, z };
        bottom.normal = glm::normalize(glm::vec3(x, 0.0f, z));
        bottom.texCoord = { (float)i / sectors, 0.0f };
        builder.addVertex(bottom);
    }

    // Side indices
    for (int i = 0; i < sectors; ++i)
    {
        int top0    = i * 2;
        int bottom0 = i * 2 + 1;
        int top1    = (i + 1) * 2;
        int bottom1 = (i + 1) * 2 + 1;

        builder.addIndex(top0);    builder.addIndex(bottom0); builder.addIndex(top1);
        builder.addIndex(bottom0); builder.addIndex(bottom1); builder.addIndex(top1);
    }

    // Cap center vertices
    int topCenterIdx    = (sectors + 1) * 2;
    int bottomCenterIdx = topCenterIdx + 1;

    StaticVertex topCenter{};
    topCenter.position = { 0.0f, halfHeight, 0.0f };
    topCenter.normal = { 0.0f, 1.0f, 0.0f };
    topCenter.texCoord = { 0.5f, 0.5f };
    builder.addVertex(topCenter);

    StaticVertex bottomCenter{};
    bottomCenter.position = { 0.0f, -halfHeight, 0.0f };
    bottomCenter.normal = { 0.0f, -1.0f, 0.0f };
    bottomCenter.texCoord = { 0.5f, 0.5f };
    builder.addVertex(bottomCenter);

    // Cap indices — reuse the side top/bottom vertices (i*2 and i*2+1)
    for (int i = 0; i < sectors; ++i)
    {
        int topA = i * 2;
        int topB = (i + 1) * 2;       // valid: loop has sectors+1 top verts
        int botA = i * 2 + 1;
        int botB = (i + 1) * 2 + 1;

        // Top cap
        builder.addIndex(topCenterIdx); builder.addIndex(topA); builder.addIndex(topB);
        // Bottom cap (winding flipped)
        builder.addIndex(bottomCenterIdx); builder.addIndex(botB); builder.addIndex(botA);
    }

    auto mesh = builder.build();
    return mesh;
}