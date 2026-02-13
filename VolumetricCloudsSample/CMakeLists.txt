set(PROJECT_NAME VolumetricCloudsSample)

# Add all source files in a folder and its subfolders
file(GLOB_RECURSE SOURCES *.cpp *.h)

# Add shared library target
add_executable(${PROJECT_NAME} ${SOURCES})

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
	NativeScripts
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Resources" DESTINATION "${CMAKE_BINARY_DIR}/${PROJECT_NAME}")

