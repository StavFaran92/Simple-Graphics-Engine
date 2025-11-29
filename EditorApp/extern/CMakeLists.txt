add_subdirectory(imgui)
add_subdirectory(ImGuizmo)

target_include_directories(Engine PUBLIC 	
	imgui
	ImGuizmo
)

target_link_libraries(Engine PUBLIC 
	ImGui
	ImGuizmo
)




