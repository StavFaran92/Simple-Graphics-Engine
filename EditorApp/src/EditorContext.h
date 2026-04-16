#pragma once

#include "sge.h"

#include "Dialogs/MaterialCreateDialog.h"
#include "Dialogs/ModelImportDialog.h"
#include "Dialogs/AnimationImportDialog.h"
#include "Dialogs/TextureImportDialog.h"
#include "Dialogs/LuaScriptImportDialog.h"
#include "Dialogs/FolderCreateDialog.h"
#include "Dialogs/AssetSelectDialog.h"
#include "Dialogs/SceneCreateDialog.h"
#include "Dialogs/ScriptSelectDialog.h"
#include "Dialogs/EntitySelectDialog.h"
#include "Dialogs/TextureSelectDialog.h"
#include "Dialogs/TextureCreateDialog.h"
#include "Dialogs/ShaderCreateDialog.h"
#include "Dialogs/LuaScriptCreateDialog.h"
#include "Dialogs/ProjectSettingsDialog.h"
#include "Dialogs/MaterialEditDialog.h"

class EditorContext
{
public:
    static EditorContext& Instance()
    {
        static EditorContext instance;
        return instance;
    }

    MaterialCreateDialog materialCreateDialog;
    ModelImportDialog modelImportDialog;
    AnimationImportDialog animationImportDialog;
    TextureImportDialog textureImportDialog;
    LuaScriptImportDialog luaScriptImportDialog;
    FolderCreateDialog folderCreateDialog;
    SceneCreateDialog sceneCreateDialog;
    ScriptSelectDialog scriptSelectDialog;
    EntitySelectDialog entitySelectDialog;
    TextureSelectDialog textureSelectDialog;
    TextureCreateDialog textureCreateDialog;
    ShaderCreateDialog shaderCreateDialog;
    LuaScriptCreateDialog luaScriptCreateDialog;
    ProjectSettingsDialog projectSettingsDialog;
    MaterialEditDialog materialEditDialog;

    EditorContext(const EditorContext&) = delete;
    EditorContext& operator=(const EditorContext&) = delete;
    EditorContext(EditorContext&&) = delete;
    EditorContext& operator=(EditorContext&&) = delete;

private:
    EditorContext() = default;
};
