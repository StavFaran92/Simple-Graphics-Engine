#include "geometry/Sphere.h"
#include "core/ApplicationConstants.h"
#include "geometry/Mesh.h"
#include "geometry/MeshBuilder.h"
#include "core/Engine.h"
#include "geometry/ModelImporter.h"
#include "core/Factory.h"
#include "memory/Assets.h"

std::shared_ptr<Mesh> Sphere::createMesh(float radius, int sectors, int stacks)
{
    //ModelImporter::ModelImportSettings settings;
    //settings.name = "SGE_SPHERE_MESH";
    //settings.isTransient = true;
    //return Engine::get()->getSubSystem<ModelImporter>()->import(SGE_ROOT_DIR "Resources/Engine/Meshes/sphere.gltf", settings).mesh;
    // clear memory of prev arrays
    auto positions = new std::vector<glm::vec3>();
    auto normals = new std::vector<glm::vec3>();
    auto texcoords = new std::vector<glm::vec2>();

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * Constants::PI / sectors;
    float stackStep = Constants::PI / stacks;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stacks; ++i)
    {
        stackAngle = Constants::PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) positions per stack
        // the first and last positions have same position and normal, but different tex coords
        for (int j = 0; j <= sectors; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            positions->push_back({ x, y, z });

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            normals->push_back({ nx, ny, nz });

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectors;
            t = (float)i / stacks;
            texcoords->push_back({ s, t });
        }
    }

    // generate CCW index list of sphere triangles
    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    auto indices = new std::vector<unsigned int>();
    std::vector<int> lineIndices;
    int k1, k2;
    for (int i = 0; i < stacks; ++i)
    {
        k1 = i * (sectors + 1);     // beginning of current stack
        k2 = k1 + sectors + 1;      // beginning of next stack

        for (int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                indices->push_back(k1);
                indices->push_back(k2);
                indices->push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stacks - 1))
            {
                indices->push_back(k1 + 1);
                indices->push_back(k2);
                indices->push_back(k2 + 1);
            }

            // store indices for lines
            // vertical lines for all stacks, k1 => k2
            lineIndices.push_back(k1);
            lineIndices.push_back(k2);
            if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
            {
                lineIndices.push_back(k1);
                lineIndices.push_back(k1 + 1);
            }
        }
    }

    

    VertexLayout layout;
    layout.attribs.push_back(LayoutAttribute::Positions);
    layout.attribs.push_back(LayoutAttribute::Normals);
    layout.attribs.push_back(LayoutAttribute::Texcoords);
    layout.numOfVertices = positions->size();
    layout.build();

    MeshBuilder::builder()
        .addPositions(*positions)
        .addNormals(*normals)
        .addTexcoords(*texcoords)
        .addIndices(*indices)
        .build();

    auto& builder = MeshBuilder::builder();
    builder.addVertex();
    builder.setMeshType(MeshType::StaticMesh);
    auto mesh = builder.build();

    return mesh;
}