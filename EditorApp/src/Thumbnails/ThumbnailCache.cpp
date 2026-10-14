#include "ThumbnailCache.h"

ImTextureID ThumbnailCache::get(const AssetRecord& aInfo)
{
	if (!m_registry.getGenerator(aInfo.aType))
		return (ImTextureID)0;

	auto iter = m_entries.find(aInfo.uuid);
	if (iter == m_entries.end())
	{
		iter = m_entries.emplace(aInfo.uuid, Entry{}).first;
		m_pending.push_back(aInfo.uuid);
	}

	Entry& entry = iter->second;
	entry.lastUsedFrame = ImGui::GetFrameCount();

	if (!entry.hasImage)
		return (ImTextureID)0;

	return (ImTextureID)(intptr_t)entry.texture->getID();
}

void ThumbnailCache::invalidate(UUID uuid)
{
	auto iter = m_entries.find(uuid);
	if (iter == m_entries.end() || iter->second.state == State::Pending)
		return;

	iter->second.state = State::Pending;
	m_pending.push_back(uuid);
}

void ThumbnailCache::update()
{
	// Evict before any window draws, so no draw list still references an evicted texture
	evict();
	generatePending();
}

void ThumbnailCache::evict()
{
	// Entries drawn last frame are on screen; evicting them would only regenerate them
	const int protectedFrame = ImGui::GetFrameCount() - 1;

	while ((int)m_entries.size() > MaxEntries)
	{
		auto oldest = m_entries.end();
		for (auto iter = m_entries.begin(); iter != m_entries.end(); ++iter)
		{
			if (iter->second.lastUsedFrame >= protectedFrame)
				continue;

			if (oldest == m_entries.end() || iter->second.lastUsedFrame < oldest->second.lastUsedFrame)
				oldest = iter;
		}

		// Everything left is on screen, so the cap is exceeded until it scrolls away
		if (oldest == m_entries.end())
			break;

		m_entries.erase(oldest);
	}
}

void ThumbnailCache::generatePending()
{
	auto assets = Engine::get()->getSubSystem<Assets>();

	int generated = 0;
	while (generated < GenerationsPerFrame && !m_pending.empty())
	{
		UUID uuid = m_pending.front();
		m_pending.pop_front();

		// Evicted since it was queued, or queued twice
		auto iter = m_entries.find(uuid);
		if (iter == m_entries.end() || iter->second.state != State::Pending)
			continue;

		if (!assets->hasAsset(uuid))
		{
			m_entries.erase(iter);
			continue;
		}

		const AssetRecord& aInfo = assets->getInfo(uuid);
		IThumbnailGenerator* generator = m_registry.getGenerator(aInfo.aType);
		if (!generator)
		{
			m_entries.erase(iter);
			continue;
		}

		Entry& entry = iter->second;
		if (entry.texture.isEmpty())
		{
			entry.texture = Texture::createTexture(ThumbnailSize, ThumbnailSize, 4,
				TextureInternalFormat::RGBA8, TextureFormat::RGBA, TextureType::UNSIGNED_BYTE);
		}

		if (generator->generate(aInfo, entry.texture))
		{
			entry.state = State::Ready;
			entry.hasImage = true;
		}
		else
		{
			// Keep the entry so the asset isn't retried every frame; invalidate() retries it
			entry.state = State::Failed;
			entry.hasImage = false;
			entry.texture = TextureResourceRef();
		}

		generated++;
	}
}
