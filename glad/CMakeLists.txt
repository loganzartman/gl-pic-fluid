project(Glad)

add_library(glad include/glad/glad.h src/glad.c)
if (UNIX)
target_link_libraries(glad dl)
endif (UNIX)
target_include_directories(glad PUBLIC include/)
