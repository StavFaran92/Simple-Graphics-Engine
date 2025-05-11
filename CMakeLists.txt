cmake_minimum_required(VERSION 3.20)
project(SGE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(Engine)
add_subdirectory(NativeScripts)
add_subdirectory(EditorApp)
add_subdirectory(PhysxSample)
add_subdirectory(NormalMapSample)
add_subdirectory(Sandbox)
add_subdirectory(ShadowSample)
add_subdirectory(OceanSample)
add_subdirectory(InstancingSample)
add_subdirectory(ShaderToy)
add_subdirectory(TerrainSample)
#add_subdirectory(PostProcessSample)
#add_subdirectory(VolumetricCloudsSample)
add_subdirectory(Game)

