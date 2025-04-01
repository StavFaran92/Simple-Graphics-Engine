//#include "Grid.h"
//
//#include "Vertex.h"
//#include "Renderer.h"
//#include "Mesh.h"
//
//std::shared_ptr<Grid> Grid::GenerateGrid(int slices)
//{
//    auto grid = std::make_shared<Grid>();
//
//    auto vertices = std::make_shared<std::vector<Vertex>>();;
//    auto indices = std::make_shared<std::vector<unsigned int>>();
//
//    for (int j = -slices; j <= slices; ++j) {
//        for (int i = -slices; i <= slices; ++i) {
//            float x = i;
//            float y = 0;
//            float z = j;
//
//            Vertex vertex;
//            vertex.Position = glm::vec3(x, y, z);
//            vertex.Normal = Constants::VEC3_ZERO;
//            vertex.TexCoords = { 0,0 };
//
//            vertices->push_back(vertex);
//        }
//    }
//
//    for (unsigned int j = 0; j < slices; ++j) {
//        for (unsigned int i = 0; i < slices; ++i) {
//
//            unsigned int row1 = j * (slices + 1);
//            unsigned int row2 = (j + 1) * (slices + 1);
//
//            indices->insert(indices->end(), { row1 + i, row1 + i + 1, row1 + i + 1, row2 + i + 1 });
//            indices->insert(indices->end(), { row2 + i + 1, row2 + i, row2 + i, row1 + i });
//
//        }
//    }
//
//    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(vertices, indices);
//
//    //auto shader = std::make_shared < Shader >("Resources\\Shaders\\shader.vert", "Resources\\Shaders\\shader.frag");
//    //shader->SetEnableMaterials(true);
//    //shader->SetEnableLights(true);
//    //shader->SetEnableTextures(true);
//    //grid->AttachShader(shader);
//
//    std::shared_ptr<Material> material = std::make_shared<Material>(32.0f);
//    grid->UseMaterial(material);
//
//    grid->m_meshes.push_back(mesh);
//
//    return grid;
//}
//
//void Grid::Draw(std::shared_ptr<IRenderer> renderer, Resource<Shader> shader)
//{
//    auto currShader = m_shader;
//
//    if (shader)
//        currShader = shader;
//
//    currShader->SetMat4("model", m_transformation->GetTransformation());
//
//    if (m_material)
//    {
//        m_material->UseMaterial(currShader);
//    }
//
//    renderer->SetDrawType(Renderer::DrawType::Lines);
//
//    for (auto i = 0; i < m_meshes.size(); i++)
//    {
//        m_meshes[i]->renderMesh(currShader, renderer);
//    }
//}
