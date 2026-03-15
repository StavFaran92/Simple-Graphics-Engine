#pragma once

#include "serialize/Archiver.h"

class ArchiveInitializer
{
public:
	static void init()
	{
        Archiver::registerSerializeFunction([]() {
            SerializedScene serializedScene = Archiver::serializeScene(Engine::get()->getContext()->getActiveScene());

            //ResourceTypeManager* manager = AssetFactory::getManager(AssetType::SCENE);
            //if (!manager)
            //{
            //    logError("No ResourceTypeManager registered for asset type {}", static_cast<int>(AssetType::SCENE));
            //    return AssetHandle<Asset>::empty;
            //}

            //if (!manager->saveResource(*desc.resourceCreateDescriptor, dest))
            //{
            //    logError("Failed to save asset type {} to: {}", static_cast<int>(type), dest.absolute().string());
            //    return AssetHandle<Asset>::empty;
            //}
            
            AssetUpdateDescriptor desc;
            desc.makeResourceUpdateDescriptor<SceneCreateDescriptor>();
            Engine::get()->getContext()->getActiveSceneAsset()->updateAsset(desc);

            //SerializedContext serializedContext = Archiver::serializeScene(Engine::get()->getContext()->getActiveScene());

            //auto projectDir = Engine::get()->getProjectDirectory();
            //std::ofstream os(projectDir + "/entities.json");
            //cereal::JSONOutputArchive oarchive(os);

            //try
            //{
            //    oarchive(serializedContext);
            //}
            //catch (const cereal::Exception& e)
            //{
            //    logError("Serialization Error occured: {}", e.what());
            //}

            //Engine::get()->getContext()->save();
            });

        Archiver::registerDeserializeFunction([]() {
            //auto projectDir = Engine::get()->getProjectDirectory();
            //std::ifstream is(projectDir + "/entities.json");
            //cereal::JSONInputArchive iarchive(is);
            //SerializedContext ptrs;

            //try
            //{
            //    iarchive(ptrs);
            //    Archiver::deserializeContext(ptrs, Engine::get()->getContext());
            //    
            //}
            //catch (const cereal::Exception& e)
            //{
            //    logError("Deserialization Error occured: {}", e.what());
            //}
            });
	}
};