#pragma once

#include "Archiver.h"

class ArchiveInitializer
{
public:
	static void init()
	{
        Archiver::registerSerializeFunction([]() {
            SerializedContext serializedContext = Archiver::serializeContext(Engine::get()->getContext());

            auto projectDir = Engine::get()->getProjectDirectory();
            std::ofstream os(projectDir + "/entities.json");
            cereal::JSONOutputArchive oarchive(os);
            oarchive(serializedContext);

            Engine::get()->getContext()->save();
            });

        Archiver::registerDeserializeFunction([]() {
            auto projectDir = Engine::get()->getProjectDirectory();
            std::ifstream is(projectDir + "/entities.json");
            cereal::JSONInputArchive iarchive(is);
            SerializedContext ptrs;
            iarchive(ptrs);

            Archiver::deserializeContext(ptrs, Engine::get()->getContext());
            });
	}
};