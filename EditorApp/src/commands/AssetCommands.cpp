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

SetSceneSkyCmd::SetSceneSkyCmd(UUID scene, UUID skyCubemap, std::string label)
	: m_scene(scene), m_sky(skyCubemap), m_label(std::move(label))
{
}

bool SetSceneSkyCmd::execute(std::string& error)
{
	SceneAssetRef scene(m_scene);
	if (scene.isEmpty())
	{
		error = "Scene " + m_scene.str() + " no longer exists";
		return false;
	}

	if (!m_sky.empty())
	{
		TextureAssetRef sky(m_sky);
		if (sky.isEmpty())
		{
			error = "No texture asset with uuid " + m_sky.str();
			return false;
		}

		TextureResourceRef texture = sky.resource();
		if (texture.isEmpty() || texture->getData().target != TextureTarget::TEXTURE_CUBE_MAP)
		{
			error = "'" + sky.info().name + "' isn't a cubemap texture";
			return false;
		}
	}

	m_previousSky = scene->environment.skyCubemap;
	scene->environment.skyCubemap = m_sky;

	// The live scene picks it up through SceneAsset::fillData on the next sync
	scene.makeDirty();
	return true;
}

void SetSceneSkyCmd::undo()
{
	SceneAssetRef scene(m_scene);
	if (scene.isEmpty())
		return;

	scene->environment.skyCubemap = m_previousSky;
	scene.makeDirty();
}
