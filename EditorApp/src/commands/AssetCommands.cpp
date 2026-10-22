#include "AssetCommands.h"

#include "sge.h"
#include "memory/Asset.h"
#include "memory/Assets.h"
#include "Thumbnails/ThumbnailCache.h"

#include <stdexcept>

CreateAssetCmd::CreateAssetCmd(AssetRef<Asset> created, std::unique_ptr<ResourceBuildDescriptor> buildDesc, std::string label)
	: m_asset(created.getUID()), m_buildDesc(std::move(buildDesc)), m_label(std::move(label))
{
	// The record's folder already includes the original target directory
	const AssetRecord& record = created.info();
	m_desc.name = record.name;
	m_desc.aType = record.aType;
	m_desc.isEngineOwned = record.isEngineOwned;
	m_desc.visibility = record.visibility;
	m_desc.assetDirectory = record.assetDirectory;
	m_desc.hintUUID = m_asset;
}

// Only reached on redo: the asset was created before this command was recorded
bool CreateAssetCmd::execute(std::string&)
{
	AssetBuildDescriptor desc = m_desc;
	if (Engine::get()->getSubSystem<Assets>()->createAsset(desc, *m_buildDesc).isEmpty())
		throw std::runtime_error("Creating asset '" + m_desc.name + "' failed; see the console");
	return true;
}

void CreateAssetCmd::undo()
{
	Engine::get()->getSubSystem<Assets>()->deleteAsset(m_asset);
	ThumbnailCache::Instance().invalidate(m_asset);
}

EditAssetCmd::EditAssetCmd(AssetSnapshot before, AssetSnapshot after, std::string label)
	: m_before(std::move(before)), m_after(std::move(after)), m_label(std::move(label))
{
}

bool EditAssetCmd::execute(std::string&)
{
	applyAssetSnapshot(m_after);
	return true;
}

void EditAssetCmd::undo()
{
	applyAssetSnapshot(m_before);
}
