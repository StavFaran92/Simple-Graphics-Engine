add_subdirectory(imgui)
add_subdirectory(ImGuizmo)
add_subdirectory(tinyfiledialogs)

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
	imgui
	ImGuizmo
	nativefiledialog/include
	tinyfiledialogs
	physx/PhysX/include
	physx/PxShared/include
	FileWatch
)

target_link_directories(Engine PUBLIC 
	assimp/lib
	glew/lib/Release/x64
	SDL2/lib
	spdlog/lib
	nativefiledialog/lib
	physx/PhysX/bin/win.x86_64.vc142.mt/checked
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
	ImGui
	nfd
	ImGuizmo
	tinyfiledialogs
)

set(OUTPUT_DIR ${CMAKE_BINARY_DIR})

set(PHYSX_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/physx/PhysX/bin/win.x86_64.vc142.mt/checked")
file(COPY "${PHYSX_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Release)
file(COPY "${PHYSX_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/RelWithDebInfo)

set(SDL_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/bin")
file(COPY "${SDL_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Release)
file(COPY "${SDL_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/RelWithDebInfo)

set(ASSIMP_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assimp/bin")
file(COPY "${ASSIMP_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Release)
file(COPY "${ASSIMP_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/RelWithDebInfo)

set(GLEW_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glew/bin")
file(COPY "${GLEW_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/Release)
file(COPY "${GLEW_BIN_DIR}/" DESTINATION ${OUTPUT_DIR}/bin/RelWithDebInfo)




