cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif()

project(gl-pic-fluid)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(fluid)
add_subdirectory(src)

# copy shaders to bin
add_custom_target(copy-shader-files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shader ${CMAKE_BINARY_DIR}/shader)

# add glm
add_subdirectory(glm)
target_link_libraries(fluid glm)

# add glad
add_subdirectory(glad)
target_link_libraries(fluid glad)

# add glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
target_link_libraries(fluid glfw)
