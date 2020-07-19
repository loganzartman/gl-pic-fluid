cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif()

project(gl-pic-fluid)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIBS glad glfw glm)
add_executable(fluid)
add_subdirectory(src)
target_link_libraries(fluid ${LIBS})
target_include_directories(fluid PUBLIC include)

# googletest and tests
add_subdirectory(googletest)
add_executable(fluid_tests test/fluid_tests.cpp)
target_link_libraries(fluid_tests ${LIBS} gtest gtest_main)

# copy shaders to bin
add_custom_target(copy-shader-files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shader ${CMAKE_BINARY_DIR}/shader)

# add glm
add_subdirectory(glm)

# add glad
add_subdirectory(glad)

# add glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

