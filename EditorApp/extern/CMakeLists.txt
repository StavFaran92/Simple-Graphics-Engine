set(CMAKE_FOLDER extern)

add_subdirectory(imgui)
add_subdirectory(ImGuizmo)
add_subdirectory(imgui-node-editor)
add_subdirectory(cpptrace)

target_include_directories(Engine PUBLIC
	imgui
	ImGuizmo
)

target_link_libraries(Engine PUBLIC
	ImGui
	ImGuizmo
)

# Used only inside the Engine DLL, for ErrorHandler's crash reports
target_link_libraries(Engine PRIVATE cpptrace::cpptrace)

target_include_directories(EditorApp PRIVATE
	imgui-node-editor
)

target_link_libraries(EditorApp PRIVATE
	imgui-node-editor
)




