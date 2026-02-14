#pragma once

#include "memory/ResourcePipeline.h"

class MeshGroupTypeManager : public ResourceTypeManager
{
	class Saver : public IResourceSaver
	{
		void save(const AssetRecord& record) override;
	};

	class Importer : public IResourceImporter
	{
		void import(const AssetRecord& record) override;
	};

	class Factory : public IAssetFactory
	{
		Asset* create(AssetCreateDescriptor& desc) override;
	};

	Saver m_saver;
	Importer m_importer;
	Factory m_factory;

public:
	IResourceParser*   getParser() override { return nullptr; }
	IResourceLoader*   getLoader(const std::string&) override { return nullptr; }
	IResourceFactory*  getFactory() override { return nullptr; }
	IResourceSaver*    getSaver(const std::string&) override { return &m_saver; }
	IResourceImporter* getImporter(const std::string&) override { return &m_importer; }
	IAssetFactory*     getAssetFactory() override { return &m_factory; }
};
