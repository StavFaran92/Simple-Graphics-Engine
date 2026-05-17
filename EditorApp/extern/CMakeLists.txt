add_subdirectory(imgui)
add_subdirectory(ImGuizmo)
add_subdirectory(imgui-node-editor)

target_include_directories(Engine PUBLIC
	imgui
	ImGuizmo
)

target_link_libraries(Engine PUBLIC
	ImGui
	ImGuizmo
)

target_include_directories(EditorApp PRIVATE
	imgui-node-editor
)

target_link_libraries(EditorApp PRIVATE
	imgui-node-editor
)




