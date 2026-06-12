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
    MeshBuilder builder;

    float sectorStep = 2.0f * Constants::PI / sectors;
    float halfHeight = height * 0.5f;

    // Side vertices: (sectors+1) pairs, top at i*2, bottom at i*2+1
    for (int i = 0; i <= sectors; ++i)
    {
        float angle = i * sectorStep;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);

        builder.addPosition({ x,  halfHeight, z });
        builder.addNormal(glm::normalize(glm::vec3(x, 0.0f, z)));
        builder.addTexcoord({ (float)i / sectors, 1.0f });

        builder.addPosition({ x, -halfHeight, z });
        builder.addNormal(glm::normalize(glm::vec3(x, 0.0f, z)));
        builder.addTexcoord({ (float)i / sectors, 0.0f });
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

    builder.addPosition({ 0.0f,  halfHeight, 0.0f });
    builder.addNormal({ 0.0f,  1.0f, 0.0f });
    builder.addTexcoord({ 0.5f, 0.5f });

    builder.addPosition({ 0.0f, -halfHeight, 0.0f });
    builder.addNormal({ 0.0f, -1.0f, 0.0f });
    builder.addTexcoord({ 0.5f, 0.5f });

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