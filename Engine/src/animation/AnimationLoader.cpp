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
}

void readSceneNodeData(MeshNodeData& nodeData, const aiNode* scene)
{
    assert(src);

    nodeData.name = scene->mName.data;
    nodeData.transformation = AssimpGLMHelpers::convertMat4ToGLMFormat(scene->mTransformation);
    nodeData.childrenCount = scene->mNumChildren;

    for (int i = 0; i < scene->mNumChildren; i++)
    {
        MeshNodeData newData;
        readSceneNodeData(newData, scene->mChildren[i]);
        nodeData.children.push_back(newData);
    }
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

Resource<Animation> AnimationLoader::load(AssetInfo aInfo)
{
    std::string filepath;
    if (aInfo.isTransient)
    {
        filepath = aInfo.filePath;
    }
    else
    {
        filepath = Engine::get()->getProjectDirectory() + aInfo.filePath;
    }

    const aiScene* scene = m_importer.ReadFile(filepath, aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        logError("ERROR::ASSIMP::{}", m_importer.GetErrorString());
        return Resource<Animation>::empty;
    }

    assert(scene && scene->mRootNode && scene->HasAnimations());


    auto aiAnimation = scene->mAnimations[0];

    MeshNodeData rootNode; //todo fix
    readSceneNodeData(rootNode, scene->mRootNode);

    std::unordered_map<std::string, std::shared_ptr<Bone>> bones;
    readAnimationBones(aiAnimation, bones);

    Animation* anim = new Animation();
    anim->build(aiAnimation->mName.C_Str(), (float)aiAnimation->mDuration, (float)aiAnimation->mTicksPerSecond, rootNode, bones);

    Engine::get()->getMemoryPool().add(aInfo.uuid, anim);
    auto& res = Resource<Animation>(aInfo.uuid);

    return res;
}

Resource<Animation> AnimationLoader::import(const std::string& path, AnimationImportSettings settings)
{
    // Assertion
    if (!std::filesystem::exists(path))
    {
        logError("File doesn't exists: " + path);
        return Resource<Animation>::empty;
    }

    // Data extract
    if (settings.name.empty())
    {
        settings.name = std::filesystem::path(path).filename().stem().string();
    }

    // Copy
    const aiScene* scene = m_importer.ReadFile(path, aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        logError("ERROR::ASSIMP::{}", m_importer.GetErrorString());
        return Resource<Animation>::empty;
    }

    assert(scene && scene->mRootNode && scene->HasAnimations());

    // Paste
    auto& projectDir = Engine::get()->getProjectDirectory();
    Assimp::Exporter exporter;
    const std::string relativeFilepath = "/" + settings.name + ".dae";
    const std::string savedFilePath = projectDir + "/" + relativeFilepath;
    exporter.Export(scene, "collada", savedFilePath);

    // Add asset to registry
    AssetInfo aInfo;
    aInfo.uuid = uuid::generate_uuid_v4();
    aInfo.aType = AssetType::ANIMATION;
    aInfo.filePath = relativeFilepath;
    aInfo.name = settings.name;

    // Load
    Resource<Animation> animation = load(aInfo);
    aInfo.data = animation;

    // Add Asset
    Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);   

    return animation;
}