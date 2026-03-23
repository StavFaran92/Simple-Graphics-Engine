#include "animation/AnimationBinaryLoader.h"

#include <fstream>

#include "animation/Animation.h"
#include "animation/Bone.h"
#include "core/Logger.h"

namespace
{
	struct AnimationBinaryHeader
	{
		uint32_t nameLength = 0;
		uint32_t bonesCount = 0;
		uint32_t nodesCount = 0;
		float duration = 0.0f;
		float ticksPerSecond = 0.0f;
	};

	struct AnimationBinaryBoneHeader
	{
		uint32_t nameLength = 0;
		uint32_t positionsCount = 0;
		uint32_t rotationsCount = 0;
		uint32_t scalesCount = 0;
	};

	struct AnimationBinaryNodeHeader
	{
		uint32_t nameLength = 0;
		int32_t childrenCount = 0;
		uint32_t childrenIndicesCount = 0;
	};

	inline bool writeAll(std::ofstream& file, const void* data, std::size_t size)
	{
		if (size == 0)
			return true;
		file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
		return static_cast<bool>(file);
	}

	inline bool readAll(std::ifstream& file, void* data, std::size_t size)
	{
		if (size == 0)
			return true;
		file.read(reinterpret_cast<char*>(data), static_cast<std::streamsize>(size));
		return static_cast<bool>(file);
	}

} // anonymous namespace

bool AnimationBinaryLoader::save(const AnimationData& animData, const std::string& targetFile)
{
	std::ofstream file(targetFile, std::ios::binary);
	if (!file.is_open())
	{
		logError("AnimationBinaryLoader::save - Failed to open file '{}' for writing", targetFile);
		return false;
	}

	AnimationBinaryHeader header{};
	header.nameLength = static_cast<uint32_t>(animData.name.size());
	header.bonesCount = static_cast<uint32_t>(animData.bones.size());
	header.nodesCount = static_cast<uint32_t>(animData.nodes.size());
	header.duration = animData.duration;
	header.ticksPerSecond = animData.ticksPerSecond;

	if (!writeAll(file, &header, sizeof(header)))
	{
		logError("AnimationBinaryLoader::save - Failed to write header to '{}'", targetFile);
		return false;
	}

	if (!writeAll(file, animData.name.data(), header.nameLength))
	{
		logError("AnimationBinaryLoader::save - Failed to write animation name to '{}'", targetFile);
		return false;
	}

	for (const auto& [boneName, bone] : animData.bones)
	{
		if (!bone)
		{
			logError("AnimationBinaryLoader::save - Found null bone '{}' while writing '{}'", boneName, targetFile);
			return false;
		}

		AnimationBinaryBoneHeader boneHeader{};
		boneHeader.nameLength = static_cast<uint32_t>(boneName.size());
		const auto& positions = bone->getPositions();
		const auto& rotations = bone->getRotations();
		const auto& scales = bone->getScales();

		boneHeader.positionsCount = static_cast<uint32_t>(positions.size());
		boneHeader.rotationsCount = static_cast<uint32_t>(rotations.size());
		boneHeader.scalesCount = static_cast<uint32_t>(scales.size());

		if (!writeAll(file, &boneHeader, sizeof(boneHeader))) return false;
		if (!writeAll(file, boneName.data(), boneHeader.nameLength)) return false;
		if (!writeAll(file, positions.data(), boneHeader.positionsCount * sizeof(KeyPosition))) return false;
		if (!writeAll(file, rotations.data(), boneHeader.rotationsCount * sizeof(KeyRotation))) return false;
		if (!writeAll(file, scales.data(), boneHeader.scalesCount * sizeof(KeyScale))) return false;
	}

	for (const MeshNodeData& node : animData.nodes)
	{
		AnimationBinaryNodeHeader nodeHeader{};
		nodeHeader.nameLength = static_cast<uint32_t>(node.name.size());
		nodeHeader.childrenCount = node.childrenCount;
		nodeHeader.childrenIndicesCount = static_cast<uint32_t>(node.children.size());

		if (!writeAll(file, &nodeHeader, sizeof(nodeHeader))) return false;
		if (!writeAll(file, node.name.data(), nodeHeader.nameLength)) return false;
		if (!writeAll(file, &node.transformation, sizeof(node.transformation))) return false;
		if (!writeAll(file, node.children.data(), nodeHeader.childrenIndicesCount * sizeof(int))) return false;
	}

	return true;
}

bool AnimationBinaryLoader::load(const std::string& sourceFile, AnimationData& outAnimData)
{
	std::ifstream file(sourceFile, std::ios::binary);
	if (!file.is_open())
	{
		logError("AnimationBinaryLoader::load - Failed to open file '{}' for reading", sourceFile);
		outAnimData = {};
		return false;
	}

	AnimationBinaryHeader header{};
	if (!readAll(file, &header, sizeof(header)))
	{
		logError("AnimationBinaryLoader::load - Failed to read header from '{}'", sourceFile);
		outAnimData = {};
		return false;
	}

	AnimationData loaded{};
	loaded.duration = header.duration;
	loaded.ticksPerSecond = header.ticksPerSecond;

	loaded.name.resize(header.nameLength);
	if (!readAll(file, loaded.name.data(), header.nameLength))
	{
		logError("AnimationBinaryLoader::load - Failed to read animation name from '{}'", sourceFile);
		outAnimData = {};
		return false;
	}

	for (uint32_t i = 0; i < header.bonesCount; ++i)
	{
		AnimationBinaryBoneHeader boneHeader{};
		if (!readAll(file, &boneHeader, sizeof(boneHeader)))
		{
			outAnimData = {};
			return false;
		}

		std::string boneName;
		boneName.resize(boneHeader.nameLength);
		if (!readAll(file, boneName.data(), boneHeader.nameLength))
		{
			outAnimData = {};
			return false;
		}

		std::vector<KeyPosition> positions(boneHeader.positionsCount);
		std::vector<KeyRotation> rotations(boneHeader.rotationsCount);
		std::vector<KeyScale> scales(boneHeader.scalesCount);

		if (!readAll(file, positions.data(), positions.size() * sizeof(KeyPosition))) { outAnimData = {}; return false; }
		if (!readAll(file, rotations.data(), rotations.size() * sizeof(KeyRotation))) { outAnimData = {}; return false; }
		if (!readAll(file, scales.data(), scales.size() * sizeof(KeyScale))) { outAnimData = {}; return false; }

		loaded.bones[boneName] = std::make_shared<Bone>(positions, rotations, scales);
	}

	loaded.nodes.resize(header.nodesCount);
	for (uint32_t i = 0; i < header.nodesCount; ++i)
	{
		AnimationBinaryNodeHeader nodeHeader{};
		if (!readAll(file, &nodeHeader, sizeof(nodeHeader))) { outAnimData = {}; return false; }

		MeshNodeData& node = loaded.nodes[i];
		node.name.resize(nodeHeader.nameLength);
		node.childrenCount = nodeHeader.childrenCount;
		node.children.resize(nodeHeader.childrenIndicesCount);

		if (!readAll(file, node.name.data(), nodeHeader.nameLength)) { outAnimData = {}; return false; }
		if (!readAll(file, &node.transformation, sizeof(node.transformation))) { outAnimData = {}; return false; }
		if (!readAll(file, node.children.data(), node.children.size() * sizeof(int))) { outAnimData = {}; return false; }
	}

	outAnimData = std::move(loaded);
	return true;
}
