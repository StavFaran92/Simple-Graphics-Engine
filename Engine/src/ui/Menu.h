#pragma once

#include "imgui.h"

#include "core/FileUtil.h"
#include "systems/Logger.h"

#include "core/Engine.h"
#include "Context.h"

#include "core/ApplicationConstants.h"

using namespace Constants;

void displayGUI();
void ShowMenuFile();
void ShowAppMainMenuBar();
void LoadModel();
void SaveFile();
void LightCreatorWindow();
void ShowModelCreatorWindow();
void ShowModelInspectorWindow();
void ShowPrimitiveCreatorWindow();