#pragma once

#include "sge.h"

#include "imgui.h"

#include <functional>
class AssetSelectDialog;

void addTextureEditWidget(AssetHandle<TextureAsset> texture, ImVec2 size, std::function<void(UUID uuid)> callback);

void addSamplerEditWidget(std::shared_ptr<TextureSampler> sampler, ImVec2 size, const std::string& name);

void displayChannelSelectWidget(int*& currentChannel);

void displayTextureWidget();

void displayColoredLabelWidget(const char* label);

bool addAssetSelectWidget(const std::string& name, AssetType aType, const std::function<void(UUID)>& onAccpetCB);

class MaterialDataWidget
{
public:
	void draw(const ResourceWrapper<Material>& mat);
};