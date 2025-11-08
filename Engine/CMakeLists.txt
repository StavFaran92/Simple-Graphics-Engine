set(PROJECT_NAME Engine)

set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

# Set relative source root (assuming src is inside Engine/)
set(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Get all source files
file(GLOB_RECURSE SOURCE_FILES
    ${SOURCE_ROOT}/*.cpp
    ${SOURCE_ROOT}/*.h
)

file(GLOB_RECURSE INCLUDE_FILES
    ${INCLUDE_ROOT}/*.h
)

# Guard against empty result
if (SOURCES STREQUAL "")
    message(FATAL_ERROR "No source files found in ${SOURCE_ROOT}")
endif()

# Visual Studio filters
foreach(SRC_FILE IN LISTS SOURCE_FILES)
    file(RELATIVE_PATH REL_PATH "${SOURCE_ROOT}" "${SRC_FILE}")
    get_filename_component(FOLDER_PATH "${REL_PATH}" PATH)
    string(REPLACE "/" "\\" GROUP_PATH "${FOLDER_PATH}")
    source_group("${GROUP_PATH}" FILES "${SRC_FILE}")
endforeach()

foreach(INC_FILE IN LISTS INCLUDE_FILES)
    file(RELATIVE_PATH REL_PATH "${INCLUDE_ROOT}" "${INC_FILE}")
    get_filename_component(FOLDER_PATH "${REL_PATH}" PATH)
    string(REPLACE "/" "\\" GROUP_PATH "${FOLDER_PATH}")
    source_group("include\\${GROUP_PATH}" FILES "${INC_FILE}")
endforeach()

# Add library
add_library(Engine SHARED ${SOURCE_FILES} ${INCLUDE_FILES})


target_precompile_headers(${PROJECT_NAME} PUBLIC src/core/pch.h)

target_include_directories(${PROJECT_NAME} PRIVATE src/)
target_include_directories(${PROJECT_NAME} PRIVATE include/)

target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_BUILD_DLL)
target_compile_options(${PROJECT_NAME} PUBLIC /MP)
target_compile_options(${PROJECT_NAME} PUBLIC /MT)

target_compile_options(${PROJECT_NAME} PUBLIC
    $<$<CONFIG:RelWithDebInfo>:/Od>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    $<$<CONFIG:RelWithDebInfo>:SGE_DEBUG>
)

if(MSVC)
    set_source_files_properties(
        ${SOURCE_ROOT}/scripts/LuaBindings.cpp
        PROPERTIES COMPILE_FLAGS "/bigobj"
    )
endif()
 
add_subdirectory(extern)



