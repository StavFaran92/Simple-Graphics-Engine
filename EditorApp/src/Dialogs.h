#pragma once

#include "sge.h"

#include <string>

void displaySelectScriptDialog(std::string& scriptName);

void displaySelectShaderDialog(std::string& uuid);

void displayEntitySelectDialog(Entity& e);

void displayTextureSelectDialog();

void displayTextureCreatorDialog();

void displayShaderCreatorDialog();

void displayLuaScriptCreatorDialog();

void displayMaterialEditDialog();

void displayAssetSelectDialog(AssetType aType, UUID& uuid);