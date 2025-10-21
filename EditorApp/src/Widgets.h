#pragma once

#include "sge.h"

#include "imgui.h"

#include <functional>

void addTextureEditWidget(int textureID, ImVec2 size, std::function<void(UUID uuid)> callback);

void addTextureEditWidget(AssetWrapper<Texture> texture, ImVec2 size, std::function<void(UUID uuid)> callback);

void addTextureEditWidget(AssetWrapper<Material> mat, const std::string& name, Texture::TextureType ttype);

void addSamplerEditWidget(AssetWrapper<Material> mat, ImVec2 size, const std::string& name, Texture::TextureType ttype);

void displayChannelSelectWidget(int*& currentChannel);

void displayTextureWidget();

void displayColoredLabelWidget(const char* label);

void addAssetSelectWidget(const std::string& name, AssetType aType, const std::function<void(UUID)>& uuid);