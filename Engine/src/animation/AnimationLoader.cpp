#include "animation/AnimationLoader.h"

#include "utils/AssimpGLMHelpers.h"
#include "core/Factory.h"
#include "runtime/Context.h"
#include "serialize/ProjectAssetRegistry.h"
#include "animation/Bone.h"
#include "memory/Assets.h"
#include "animation/Animation.h"
#include <filesystem>

AnimationLoader::AnimationLoader()
{
    Engine::get()->registerSubSystem<AnimationLoader>(this);

    m_importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    m_importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_TEXTURES, false);
}

int readSceneNodeData(const aiNode* scene, std::vector<MeshNodeData>& nodes)
{
    assert(src);

    int nodeID = nodes.size();

    MeshNodeData nodeData;
    nodeData.name = scene->mName.data;
    nodeData.transformation = AssimpGLMHelpers::convertMat4ToGLMFormat(scene->mTransformation);
    nodeData.childrenCount = scene->mNumChildren;
    nodes.push_back(nodeData);

    for (int i = 0; i < scene->mNumChildren; i++)
    {
        int childID = readSceneNodeData(scene->mChildren[i], nodes);
        nodes[nodeID].children.push_back(childID);
    }

    return nodeID;
    
}

void readAnimationBones(const aiAnimation* animation, std::unordered_map<std::string, std::shared_ptr<Bone>>& bones)
{
    unsigned int size = animation->mNumChannels;

    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        std::vector<KeyPosition> positions;
        for (int i = 0; i < channel->mNumPositionKeys; i++)
        {
            KeyPosition keyPosition;
            keyPosition.timeStamp = channel->mPositionKeys[i].mTime;
            keyPosition.position = AssimpGLMHelpers::convertVec3ToGLMFormat(channel->mPositionKeys[i].mValue);
            positions.push_back(keyPosition);
        }

        std::vector<KeyRotation> rotations;
        for (int i = 0; i < channel->mNumRotationKeys; i++)
        {
            KeyRotation keyRotation;
            keyRotation.timeStamp = channel->mRotationKeys[i].mTime;
            keyRotation.orientation = AssimpGLMHelpers::convertQuatToGLMFormat(channel->mRotationKeys[i].mValue);
            rotations.push_back(keyRotation);
        }

        std::vector<KeyScale> scalings;
        for (int i = 0; i < channel->mNumScalingKeys; i++)
        {
            KeyScale keyScale;
            keyScale.timeStamp = channel->mScalingKeys[i].mTime;
            keyScale.scale = AssimpGLMHelpers::convertVec3ToGLMFormat(channel->mScalingKeys[i].mValue);
            scalings.push_back(keyScale);
        }
        auto& bone = std::make_shared<Bone>(positions, rotations, scalings);

        bones[boneName] = bone;
    }
}

bool AnimationLoader::parseAnimation(const std::string& filepath, AnimationInfo& outAnimInfo)
{
    // Validate
    if (!std::filesystem::exists(filepath))
    {
        logError("File doesn't exists: " + filepath);
        return false;
    }

    const aiScene* scene = m_importer.ReadFile(filepath, aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        logError("Failed to load animation: {}", m_importer.GetErrorString());
        return false;
    }

    assert(scene && scene->mRootNode && scene->HasAnimations());


    auto aiAnimation = scene->mAnimations[0];

    std::vector<MeshNodeData> nodes; //todo fix
    readSceneNodeData(scene->mRootNode, nodes);

    std::unordered_map<std::string, std::shared_ptr<Bone>> bones;
    readAnimationBones(aiAnimation, bones);

    outAnimInfo.m_nodes = nodes;
    outAnimInfo.m_bones = bones;
    outAnimInfo.m_duration = (float)aiAnimation->mDuration;
    outAnimInfo.m_ticksPerSecond = (float)aiAnimation->mTicksPerSecond;
    outAnimInfo.m_name = aiAnimation->mName.C_Str();

    return true;
}
