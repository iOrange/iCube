cmake_minimum_required(VERSION 3.1)

project(iCube CXX)
set(CMAKE_CXX_STANDARD 17)

set(SOURCES_ROOT ${CMAKE_SOURCE_DIR}/src)
set(LIBS_ROOT ${CMAKE_SOURCE_DIR}/libs)

# building glfw
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW_BUILD_EXAMPLES" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW_BUILD_TESTS" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW_BUILD_DOCS" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "GLFW_INSTALL" FORCE)
add_subdirectory("${LIBS_ROOT}/glfw")

# building glad
add_subdirectory("${LIBS_ROOT}/glad")

# building imgui
add_subdirectory("${LIBS_ROOT}/imgui")

# building nfd
add_subdirectory("${LIBS_ROOT}/nfd")

# building bc7enc
add_subdirectory("${LIBS_ROOT}/bc7enc16")

file(GLOB_RECURSE HEADERS "${SOURCES_ROOT}/*.h")
file(GLOB_RECURSE SOURCES "${SOURCES_ROOT}/*.cpp")

include_directories(
    "${LIBS_ROOT}/glfw/include"
    "${LIBS_ROOT}/glad/include"
    "${LIBS_ROOT}/imgui"
    "${LIBS_ROOT}/nfd/src/include"
    "${LIBS_ROOT}/glm"
    "${LIBS_ROOT}/bc7enc16"
    "${LIBS_ROOT}/stb"
    "src"
)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

# an attempt to generate proper MSVC filters
# https://stackoverflow.com/questions/33808087/cmake-how-to-create-visual-studio-filters
set(SOURCES_AND_HEADERS ${HEADERS} ${SOURCES})
foreach(_source IN ITEMS ${SOURCES_AND_HEADERS})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${SOURCES_ROOT}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

target_link_libraries(${PROJECT_NAME} glfw glad dear_imgui nfd bc7enc)
