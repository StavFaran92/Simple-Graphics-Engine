#include "MeshSerializer.h"

#include <fstream>
#include "Logger.h"

#include "Mesh.h"

struct BinaryLayoutInfo
{
    int numOfPositions;
    int numOfNormals;
    int numOfTangents;
    int numOfColors;
    int numOfTexcoords;
    int numOfIndices;
    int numOfAttributes;
    int numOfBoneIDs;
    int numOfBoneWeights;
};


void MeshSerializer::writeDataToBinaryFile(const MeshData& meshData, const std::string& filename)
{
    // Create an output file stream
    std::ofstream file(filename, std::ios::binary);

    if (file.is_open()) 
    {
        BinaryLayoutInfo layoutInfo;

        layoutInfo.numOfPositions = meshData.m_positions.size();
        layoutInfo.numOfNormals = meshData.m_normals.size();
        layoutInfo.numOfTangents = meshData.m_tangents.size();
        layoutInfo.numOfColors = meshData.m_colors.size();
        layoutInfo.numOfTexcoords = meshData.m_texCoords.size();
        layoutInfo.numOfIndices = meshData.m_indices.size();
        layoutInfo.numOfBoneIDs = meshData.bonesIDs.size();
        layoutInfo.numOfBoneWeights = meshData.bonesWeights.size();
        layoutInfo.numOfAttributes = meshData.m_layout.attribs.size();

        // Write layout info
        file.write(reinterpret_cast<const char*>(&layoutInfo), sizeof(BinaryLayoutInfo));

        // Write vertex data
        file.write(reinterpret_cast<const char*>(meshData.m_positions.data()), meshData.m_positions.size() * sizeof(glm::vec3));
        file.write(reinterpret_cast<const char*>(meshData.m_normals.data()), meshData.m_normals.size() * sizeof(glm::vec3));
        file.write(reinterpret_cast<const char*>(meshData.m_tangents.data()), meshData.m_tangents.size() * sizeof(glm::vec3));
        file.write(reinterpret_cast<const char*>(meshData.m_colors.data()), meshData.m_colors.size() * sizeof(glm::vec3));
        file.write(reinterpret_cast<const char*>(meshData.m_texCoords.data()), meshData.m_texCoords.size() * sizeof(glm::vec2));
        file.write(reinterpret_cast<const char*>(meshData.bonesIDs.data()), meshData.bonesIDs.size() * sizeof(glm::ivec3));
        file.write(reinterpret_cast<const char*>(meshData.bonesWeights.data()), meshData.bonesWeights.size() * sizeof(glm::vec3));

        // Write indices
        file.write(reinterpret_cast<const char*>(meshData.m_indices.data()), meshData.m_indices.size() * sizeof(unsigned int));

        file.write(reinterpret_cast<const char*>(meshData.m_layout.attribs.data()), meshData.m_layout.attribs.size() * sizeof(LayoutAttribute));

        // Close the file
        file.close();
    }
    else {
        // Failed to open the file
        logError("Error: Failed to open file for writing: " + filename);
    }
}

void MeshSerializer::readDataFromBinaryFile(const std::string& filename, MeshData& meshData)
{
    BinaryLayoutInfo layoutInfo;

    // Open the file for reading
    std::ifstream file(filename, std::ios::binary);

    if (file.is_open()) 
    {
        // Read layout info
        file.read(reinterpret_cast<char*>(&layoutInfo), sizeof(BinaryLayoutInfo));

        // Resize vectors to hold data
        meshData.m_positions.resize(layoutInfo.numOfPositions);
        meshData.m_normals.resize(layoutInfo.numOfNormals);
        meshData.m_tangents.resize(layoutInfo.numOfTangents);
        meshData.m_colors.resize(layoutInfo.numOfColors);
        meshData.m_texCoords.resize(layoutInfo.numOfTexcoords);
        meshData.m_indices.resize(layoutInfo.numOfIndices);
        meshData.bonesIDs.resize(layoutInfo.numOfBoneIDs);
        meshData.bonesWeights.resize(layoutInfo.numOfBoneWeights);
        meshData.m_layout.attribs.resize(layoutInfo.numOfAttributes);

        // Read vertices
        file.read(reinterpret_cast<char*>(meshData.m_positions.data()), meshData.m_positions.size() * sizeof(glm::vec3));
        file.read(reinterpret_cast<char*>(meshData.m_normals.data()), meshData.m_normals.size() * sizeof(glm::vec3));
        file.read(reinterpret_cast<char*>(meshData.m_tangents.data()), meshData.m_tangents.size() * sizeof(glm::vec3));
        file.read(reinterpret_cast<char*>(meshData.m_colors.data()), meshData.m_colors.size() * sizeof(glm::vec3));
        file.read(reinterpret_cast<char*>(meshData.m_texCoords.data()), meshData.m_texCoords.size() * sizeof(glm::vec2));
        file.read(reinterpret_cast<char*>(meshData.bonesIDs.data()), meshData.bonesIDs.size() * sizeof(glm::ivec3));
        file.read(reinterpret_cast<char*>(meshData.bonesWeights.data()), meshData.bonesWeights.size() * sizeof(glm::vec3));

        // Read indices
        file.read(reinterpret_cast<char*>(meshData.m_indices.data()), meshData.m_indices.size() * sizeof(unsigned int));

        file.read(reinterpret_cast<char*>(meshData.m_layout.attribs.data()), meshData.m_layout.attribs.size() * sizeof(LayoutAttribute));

        // Close the file
        file.close();
    }
    else 
    {
        // Failed to open the file
        logError("Error: Failed to open file for reading: " + filename);
    }
}
