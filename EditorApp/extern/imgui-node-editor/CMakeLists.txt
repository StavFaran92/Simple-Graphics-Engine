set(PROJECT_NAME imgui-node-editor)

set(SOURCES
    crude_json.cpp
    imgui_canvas.cpp
    imgui_node_editor.cpp
    imgui_node_editor_api.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_compile_options(${PROJECT_NAME} PUBLIC /MT)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../imgui/
)

target_link_libraries(${PROJECT_NAME} PUBLIC ImGui)
