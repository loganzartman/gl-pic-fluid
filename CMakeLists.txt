cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif()

project(sand-in-my-fans)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(fans)
add_subdirectory(src)

# add glm
add_subdirectory(glm)
target_link_libraries(fans glm)

# add glad
add_subdirectory(glad)
target_link_libraries(fans glad)

# add glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
target_link_libraries(fans glfw)

# find glm
#find_package(glm REQUIRED)
#include_directories(${GLM_INCLUDE_DIRS})
#target_link_libraries(fans ${GLM_LIBRARIES})

# find glfw
#find_package(PkgConfig REQUIRED)
#pkg_search_module(GLFW REQUIRED glfw3)
#include_directories(${GLFW_INCLUDE_DIRS})
#target_link_libraries(fans ${GLFW_STATIC_LIBRARIES})
