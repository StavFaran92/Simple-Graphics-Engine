#pragma once

#include "sge.h"

#include <string>

// Create
void displayTextureCreatorDialog();
void displayShaderCreatorDialog();
void displayLuaScriptCreatorDialog();

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
void displayModelImportDialog();
void displayAnimationImportDialog();
void displayLuaScriptImportDialog();