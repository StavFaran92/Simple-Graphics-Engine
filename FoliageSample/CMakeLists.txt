set(PROJECT_NAME FoliageSample)

# Add all source files in a folder and its subfolders
file(GLOB_RECURSE SOURCES *.cpp *.h)

# Add shared library target
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE GUI)

target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_SOURCE_DIR}/Engine/include
	${CMAKE_SOURCE_DIR}/NativeScripts
)

target_link_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	Engine
	ImGui
)

target_compile_options(${PROJECT_NAME} PRIVATE /MP)