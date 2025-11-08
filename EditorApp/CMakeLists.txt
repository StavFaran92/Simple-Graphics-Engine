set(PROJECT_NAME EditorApp)

set(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Add all source files in a folder and its subfolders
# Get all source files
file(GLOB_RECURSE SOURCES
    ${SOURCE_ROOT}/*.cpp
    ${SOURCE_ROOT}/*.h
)

if (SOURCES STREQUAL "")
    message(FATAL_ERROR "No source files found in ${SOURCE_ROOT}")
endif()


# Add shared library target
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE GUI)
target_compile_definitions(${PROJECT_NAME} PRIVATE SGE_EDITOR)
target_compile_definitions(${PROJECT_NAME} PRIVATE SGE_SOLUTION_DIR="${CMAKE_BINARY_DIR}")

target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_SOURCE_DIR}/Engine/include
	${CMAKE_SOURCE_DIR}/Engine/src
	${CMAKE_SOURCE_DIR}/NativeScripts
)

target_link_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	Engine
	ImGui
	NativeScripts
)


