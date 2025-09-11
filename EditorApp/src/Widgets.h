#pragma once

#include "sge.h"

#include "imgui.h"

#include <functional>

void addTextureEditWidget(int textureID, ImVec2 size, std::function<void(std::string uuid)> callback);

void addTextureEditWidget(Resource<Texture> texture, ImVec2 size, std::function<void(std::string uuid)> callback);

void addTextureEditWidget(Resource<Material> mat, const std::string& name, Texture::TextureType ttype);

void addSamplerEditWidget(Resource<Material> mat, ImVec2 size, const std::string& name, Texture::TextureType ttype);

void displayChannelSelectWidget(int*& currentChannel);