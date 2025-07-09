#pragma once

#include "imgui.h"

#include "FileUtil.h"
#include "Logger.h"

#include "Engine.h"
#include "Context.h"

#include "ApplicationConstants.h"

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