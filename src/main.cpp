#include <iostream>
#include <sstream>
#include <exception>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Game.hpp"

/**
* Called in the event that any GLFW function fails.
*/
void glfw_error_callback(int error, const char* description) {
    std::stringstream str;
    str << "GLFW error " << error << ": " << description;
    throw std::runtime_error(str.str());
}

void GLAPIENTRY
MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }
    if (type == 0x824c) {
        // hack to get actual compile errors for shaders (program objects check their own errors)
        return;
    }
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "ERROR!" : ""),
        type, severity, message);
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        throw std::runtime_error("GL High Severity error");
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_P) {
            game->particles_visible = !game->particles_visible;
        }
        if (key == GLFW_KEY_G) {
            game->grid_visible = !game->grid_visible;
        }
        if (key == GLFW_KEY_SPACE) {
            game->running = !game->running;
        }
        if (key == GLFW_KEY_S) {
            game->do_step = true;
        }

        if (key == GLFW_KEY_1) {
            game->grid_display_mode = 0;
        }
        if (key == GLFW_KEY_2) {
            game->grid_display_mode = 1;
        }
        if (key == GLFW_KEY_3) {
            game->grid_display_mode = 2;
        }
        if (key == GLFW_KEY_4) {
            game->grid_display_mode = 3;
        }
        if (key == GLFW_KEY_5) {
            game->grid_display_mode = 4;
        }
    }
}

void MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    game->mouse_pos = glm::vec2(mouse_x, mouse_y);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_1) {
        if (action == GLFW_PRESS) {
            game->mouse_right_dragging = true;
            game->mouse_right_drag_start = game->mouse_pos;
        }
        if (action == GLFW_RELEASE) {
            game->mouse_right_dragging = false;
        }
    }
}

void FramebufferSizeCallback(GLFWwindow* window, int w, int h) {
}

int main() {
    // setup glfw
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) { throw std::runtime_error("glfwInit failed"); }

    // create window and GL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(800, 800, "gl-pic-fluid", NULL, NULL);
    if (!window) { throw std::runtime_error("glfwCreateWindow failed"); }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    std::cout << "** GL Version: " << GLVersion.major << "." << GLVersion.minor << std::endl;

    glfwSwapInterval(1); // enable vsync (0 to disable)

    // Callback functions
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MousePosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    // Enable GL error/debug reporting
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);

    Game game(window);
    glfwSetWindowUserPointer(window, &game);

    // main loop
    game.init();
    while (!glfwWindowShouldClose(window)) {
        game.update();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
