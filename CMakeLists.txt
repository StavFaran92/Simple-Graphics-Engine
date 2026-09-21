cmake_minimum_required(VERSION 3.20)
project(SGE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# The prebuilt third party libs (PhysX, assimp, spdlog, SDL2, ...) are all release CRT (/MT),
# so every config - Debug included - has to use the same runtime to link against them.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

# PhysX requires exactly one of NDEBUG/_DEBUG, and we link its checked (NDEBUG) binaries in
# every config, so Debug takes NDEBUG too - CMake does not define either one for Debug.
add_compile_definitions($<$<CONFIG:Debug>:NDEBUG>)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Engine root directory at configure time
add_compile_definitions(SGE_ROOT_DIR="${CMAKE_SOURCE_DIR}/Engine/")

add_subdirectory(Engine)
add_subdirectory(NativeScripts)
add_subdirectory(EditorApp)
add_subdirectory(PhysxSample)
add_subdirectory(NormalMapSample)
add_subdirectory(Sandbox)
add_subdirectory(ShadowSample)
add_subdirectory(OceanSample)
add_subdirectory(FoliageSample)
add_subdirectory(InstancingSample)
add_subdirectory(ShaderToy)
add_subdirectory(TerrainSample)
add_subdirectory(PostProcessSample)
add_subdirectory(VolumetricCloudsSample)
add_subdirectory(Game)
add_subdirectory(Testbed)

