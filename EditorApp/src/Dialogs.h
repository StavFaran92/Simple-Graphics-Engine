#pragma once

#include "sge.h"

#include <string>

void displaySelectMeshDialog(std::string& uuid);

void displaySelectAnimationDialog(std::string& uuid);

void displaySelectScriptDialog(std::string& scriptName);

void displaySelectShaderDialog(std::string& uuid);

void displayEntitySelectDialog(Entity& e);

void displayAssetTextureSelectDialog();