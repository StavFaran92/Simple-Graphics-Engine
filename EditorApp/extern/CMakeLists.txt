add_subdirectory(imgui)
add_subdirectory(ImGuizmo)
add_subdirectory(tinyfiledialogs)

target_include_directories(Engine PUBLIC 	
	imgui
	ImGuizmo
	tinyfiledialogs
)

target_link_libraries(Engine PUBLIC 
	ImGui
	ImGuizmo
	tinyfiledialogs
)




