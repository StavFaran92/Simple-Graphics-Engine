#pragma once

#include <string>
#include <functional>

#include "sge.h"

#include "EditorTool.h"
class DialogBase;

class EditorState {
public:
    // Access the singleton instance
    static EditorState& Instance() {
        static EditorState instance;
        return instance;
    }

    void init();
    void update();

    bool showLightCreatorWindow = false;
    bool showModelInspectorWindow = false;
    bool showPrimitiveCreatorWindow = false;
    bool showMeshSelector = false;
    bool showAnimationSelector = false;
    bool selectedEntityRename = false;
    bool showScriptSelector = false;
    bool showShaderSelector = false;
    bool showSamplerEditWindow = false;
    bool showTextureCreateWindow = false;
    bool showShaderCreateWindow = false;
    bool showMaterialEditWindow = false;
    bool startButtonPressed = false;
    bool isMouseInSceneView = false;
    bool showTextureDisplayWindow = false;
    bool showLuaScriptCreateWindow = false;
    bool showSceneCreateWindow = false;
    bool showSettingsWindow = false;
    bool showTextureImportWindow = false;
    bool showModelImportWindow = false;
    bool showAnimationImportWindow = false;
    bool showLuaScriptImportWindow = false;
    bool showMaterialCreateWindow = false;

    bool showAssetSelectorWindow = false;
    AssetType assetSelectType;
    std::function<void(UUID uuid)> assetSelectCB;

    std::string selectedTextureName;

    bool getState(const std::string& state);
    bool setState(const std::string& state, bool enabled);
    void addDialogDisplay(DialogBase* dialog);
    void displayDialogs();

    void setActiveEditorTool(EditorTool::Type tool);
    EditorTool::Type getActiveToolType() const;
    std::shared_ptr<EditorTool> getActiveEditorTool();

    void selectAssetForEdit(const AssetRef<Asset>& asset);
    AssetRef<Asset> getSelectedAsset() const;
    void clearAssetSelection();
    

    std::function<void(UUID uuid)> assetTextureSelectCB;
    std::function<void(Entity e)> entitySelectCB;
    std::function<void(const std::string& scriptName)> scriptSelectCB;
    TextureAssetRef selectedAssetTexture;

    std::shared_ptr<TextureSamplerAsset> selectedSampler;
    std::shared_ptr<TextureSamplerAsset> previousSampler;

    MaterialAssetRef selectedMaterialForEdit;

    

    struct Rect2D {
        glm::vec2 min;
        glm::vec2 max;

        bool contains(const glm::vec2& p) const
        {
            return
                p.x >= min.x && p.x <= max.x &&
                p.y >= min.y && p.y <= max.y;
        }
    };

    Rect2D sceneViewRect;

    WorkingDirectory& getWorkingDir();
    //void setWorkingDir(std::filesystem::path path);

    // Delete copy/move constructors to enforce singleton
    EditorState(const EditorState&) = delete;
    EditorState& operator=(const EditorState&) = delete;
    EditorState(EditorState&&) = delete;
    EditorState& operator=(EditorState&&) = delete;

private:
    static std::shared_ptr<EditorTool> getTool(EditorTool::Type type);

private:
    // Private constructor
    EditorState() = default;

    std::unordered_map<std::string, bool> m_states;

    WorkingDirectory cwd;
    std::vector<DialogBase*> m_dialogs;

    AssetRef<Asset> currentAssetEdit;
    ResourceRef<Resource> resourceCache;

    EditorTool::Type m_activeEditorToolType = EditorTool::Type::None;
    std::shared_ptr<EditorTool> m_activeEditorTool;
};