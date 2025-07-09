set(PROJECT_NAME Engine)

set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

# Add all source files in a folder and its subfolders
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)



# Add shared library target
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src/)
target_include_directories(${PROJECT_NAME} PRIVATE src/core)
target_include_directories(${PROJECT_NAME} PRIVATE src/geometry)
target_include_directories(${PROJECT_NAME} PRIVATE src/animation)
target_include_directories(${PROJECT_NAME} PRIVATE src/physics)
target_include_directories(${PROJECT_NAME} PRIVATE src/render)

target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_BUILD_DLL)
target_compile_options(${PROJECT_NAME} PUBLIC /MP)
target_compile_options(${PROJECT_NAME} PRIVATE "/Od") #todo fix
target_compile_options(${PROJECT_NAME} PUBLIC /MT)
 
add_subdirectory(extern)



