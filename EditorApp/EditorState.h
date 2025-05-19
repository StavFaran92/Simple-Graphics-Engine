#pragma once

class EditorState {
public:
    // Access the singleton instance
    static EditorState& Instance() {
        static EditorState instance;
        return instance;
    }

    // Example field
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