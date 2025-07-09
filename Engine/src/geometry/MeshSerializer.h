#pragma once

#include <vector>
#include <string>

struct MeshData;
class BinaryLayoutInfo;

class MeshSerializer
{
public:
	static void writeDataToBinaryFile(const MeshData& meshData, const std::string& filename);
	static void readDataFromBinaryFile(const std::string& filename, MeshData& meshData);

};