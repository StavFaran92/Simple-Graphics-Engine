add_subdirectory(tinyfiledialogs)
add_subdirectory(MikkTSpace)
add_subdirectory(tinyexr)

# FreeType's cmake_minimum_required predates CMP0091, which MSVC_RUNTIME_LIBRARY needs
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
add_subdirectory(freetype)
add_library(Freetype::Freetype ALIAS freetype)

set(MSDF_ATLAS_BUILD_STANDALONE OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_USE_VCPKG OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_USE_SKIA OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_NO_ARTERY_FONT ON CACHE BOOL "" FORCE)
set(MSDFGEN_DISABLE_PNG ON CACHE BOOL "" FORCE)
add_subdirectory(msdf-atlas-gen)

set_target_properties(freetype msdfgen-core msdfgen-ext msdf-atlas-gen PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")
foreach(TEXT_LIB freetype msdfgen-core msdfgen-ext msdf-atlas-gen)
	target_compile_options(${TEXT_LIB} PRIVATE /MP)
endforeach()

target_link_libraries(Engine PRIVATE msdf-atlas-gen::msdf-atlas-gen)

target_include_directories(Engine PUBLIC 	
	assimp/include
	entt
	glew/include
	GLM
	SDL2/include
	SDL2/include/SDL2
	spdlog/include
	cereal/include
	json
	stb_image
	nativefiledialog/include
	tinyfiledialogs
	physx/PhysX/include
	physx/PxShared/include
	FileWatch
	sol2/include
	lua/include
	tinyexr
	MikkTSpace
)

target_link_directories(Engine PUBLIC 
	assimp/lib
	glew/lib/Release/x64
	SDL2/lib
	spdlog/lib
	nativefiledialog/lib
	physx/PhysX/bin/win.x86_64.vc142.mt/checked
	MikkTSpace
	lua
)

target_link_libraries(Engine PUBLIC 
	PhysX_64
	PhysXCommon_64
	PhysXExtensions_static_64
	PhysXCooking_64
	PhysXCharacterKinematic_static_64
	PhysXPvdSDK_static_64
	PhysXVehicle_static_64
	PhysXFoundation_64
	SDL2
	SDL2main
	opengl32
	glew32
	assimp-vc142-mt
	spdlog
	nfd
	tinyfiledialogs
	lua54
	MikkTSpace
	tinyexr
)

set(OUTPUT_DIR ${CMAKE_BINARY_DIR})

set(PHYSX_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/physx/PhysX/bin/win.x86_64.vc142.mt/checked")
file(COPY "${PHYSX_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Release)
file(COPY "${PHYSX_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/RelWithDebInfo)
file(COPY "${PHYSX_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Debug)

set(SDL_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/bin")
file(COPY "${SDL_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Release)
file(COPY "${SDL_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/RelWithDebInfo)
file(COPY "${SDL_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Debug)

set(ASSIMP_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assimp/bin")
file(COPY "${ASSIMP_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Release)
file(COPY "${ASSIMP_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/RelWithDebInfo)
file(COPY "${ASSIMP_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Debug)

set(GLEW_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glew/bin")
file(COPY "${GLEW_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Release)
file(COPY "${GLEW_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/RelWithDebInfo)
file(COPY "${GLEW_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Debug)

set(LUA_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lua")
file(COPY "${LUA_BIN_DIR}/lua54.dll" DESTINATION ${OUTPUT_DIR}/bin/Release)
file(COPY "${LUA_BIN_DIR}/lua54.dll" DESTINATION ${OUTPUT_DIR}/bin/RelWithDebInfo)
file(COPY "${LUA_BIN_DIR}/lua54.dll" DESTINATION ${OUTPUT_DIR}/bin/Debug)




