#pragma once

#include "sge.h"

#include <string>
#include "Widgets.h"

// Create
void displayTextureCreatorDialog();
void displayShaderCreatorDialog();
void displayLuaScriptCreatorDialog();
void displayMaterialCreatorDialog();

// Edit
void displayMaterialEditDialog();

// Select
void displayTextureSelectDialog();
void displayAssetSelectDialog(AssetType aType, UUID& uuid);
void displayEntitySelectDialog(Entity& e);
void displaySelectShaderDialog(UUID& uuid);
void displaySelectScriptDialog(std::string& scriptName);

void displayProjectSettingsDialog();

// Import
void displayTextureImportDialog();
void displayAnimationImportDialog();
void displayLuaScriptImportDialog();