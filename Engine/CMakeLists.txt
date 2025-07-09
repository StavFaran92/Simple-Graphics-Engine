set(PROJECT_NAME Engine)

set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

# Set relative source root (assuming src is inside Engine/)
set(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Get all source files
file(GLOB_RECURSE SOURCES
    ${SOURCE_ROOT}/*.cpp
    ${SOURCE_ROOT}/*.h
)

# Guard against empty result
if (SOURCES STREQUAL "")
    message(FATAL_ERROR "No source files found in ${SOURCE_ROOT}")
endif()

# Visual Studio filters
foreach(SRC_FILE IN LISTS SOURCES)
    file(RELATIVE_PATH REL_PATH "${SOURCE_ROOT}" "${SRC_FILE}")
    get_filename_component(FOLDER_PATH "${REL_PATH}" PATH)
    string(REPLACE "/" "\\" GROUP_PATH "${FOLDER_PATH}")
    source_group("${GROUP_PATH}" FILES "${SRC_FILE}")
endforeach()

# Add library
add_library(Engine SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src/)
target_include_directories(${PROJECT_NAME} PRIVATE include/)

target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_BUILD_DLL)
target_compile_options(${PROJECT_NAME} PUBLIC /MP)
target_compile_options(${PROJECT_NAME} PRIVATE "/Od") #todo fix
target_compile_options(${PROJECT_NAME} PUBLIC /MT)
 
add_subdirectory(extern)



