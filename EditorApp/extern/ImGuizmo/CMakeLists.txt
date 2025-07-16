file(GLOB IMGUIZMO_SOURCES *.cpp *.h)
add_library("ImGuizmo" STATIC ${IMGUIZMO_SOURCES})
target_compile_options("ImGuizmo" PUBLIC /MT)
target_include_directories("ImGuizmo" PUBLIC 
	../imgui/
)