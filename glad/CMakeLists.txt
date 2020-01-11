project(Glad)

add_library(glad include/glad/glad.h src/glad.c)
target_link_libraries(glad dl)
target_include_directories(glad PUBLIC include/)

