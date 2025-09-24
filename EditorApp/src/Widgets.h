#pragma once

#include "sge.h"

#include "imgui.h"

#include <functional>

void addTextureEditWidget(int textureID, ImVec2 size, std::function<void(std::string uuid)> callback);

void addTextureEditWidget(ResourceWrapper<Texture> texture, ImVec2 size, std::function<void(std::string uuid)> callback);

void addTextureEditWidget(ResourceWrapper<Material> mat, const std::string& name, Texture::TextureType ttype);

void addSamplerEditWidget(ResourceWrapper<Material> mat, ImVec2 size, const std::string& name, Texture::TextureType ttype);

void displayChannelSelectWidget(int*& currentChannel);

void displayTextureWidget();

void displayColoredLabelWidget(const char* label);

void addAssetSelectWidget(const std::string& name, AssetType aType, const std::function<void(UUID)>& uuid);