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

// Button-sized asset slot: thumbnail + name when set, a dashed stencil of the expected type when empty.
// Clicking it opens the asset select dialog. Returns true when clicked.
bool addAssetSelectWidget(const char* id, UUID current, AssetType aType, const std::function<void(UUID)>& onAccpetCB);
void addEntitySelectWidget(const std::string& label, Entity current, const std::function<void(Entity)>& onAcceptCB);

class MaterialDataWidget
{
public:
	void draw(MaterialData& mat, const std::function<void(const MaterialData&)>& onChangedCB);
};