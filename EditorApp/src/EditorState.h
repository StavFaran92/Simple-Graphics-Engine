#pragma once

class EditorState {
public:
    // Access the singleton instance
    static EditorState& Instance() {
        static EditorState instance;
        return instance;
    }

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
    bool startButtonPressed = false;
    bool isMouseInSceneView = false;

    // Delete copy/move constructors to enforce singleton
    EditorState(const EditorState&) = delete;
    EditorState& operator=(const EditorState&) = delete;
    EditorState(EditorState&&) = delete;
    EditorState& operator=(EditorState&&) = delete;

private:
    // Private constructor
    EditorState() = default;
};