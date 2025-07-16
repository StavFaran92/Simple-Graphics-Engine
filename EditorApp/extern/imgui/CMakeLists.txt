file(GLOB IMGUI_SOURCES *.cpp *.h)
add_library("ImGui" STATIC ${IMGUI_SOURCES})
target_compile_options("ImGui" PUBLIC /MT)
target_include_directories("ImGui" PUBLIC 
	../SDL2/include/SDL2
)