#pragma once

#include "sge.h"

#include "imgui.h"

#include <functional>
class AssetSelectDialog;

void addTextureEditWidget(const std::string& name, TextureAssetRef texture, ImVec2 size, std::function<void(UUID uuid)> callback);

void addSamplerEditWidget(std::shared_ptr<TextureSamplerAsset> sampler, ImVec2 size, const std::string& name, const std::function<void(UUID)>& onAccpetCB = {});

void displayChannelSelectWidget(int*& currentChannel);

void displayTextureWidget();

void displayColoredLabelWidget(const char* label);

bool addAssetSelectWidget(const std::string& name, AssetType aType, const std::function<void(UUID)>& onAccpetCB);

class MaterialDataWidget
{
public:
	void draw(MaterialData& mat, const std::function<void(const MaterialData&)>& onChangedCB);
};