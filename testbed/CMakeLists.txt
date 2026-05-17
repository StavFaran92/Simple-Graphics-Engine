set(PROJECT_NAME Testbed)

file(GLOB_RECURSE SOURCES *.cpp *.h)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine/src
    ${CMAKE_SOURCE_DIR}/Engine/include
    ${CMAKE_SOURCE_DIR}/Testbed
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Engine
)

target_compile_options(${PROJECT_NAME} PRIVATE /MP)
