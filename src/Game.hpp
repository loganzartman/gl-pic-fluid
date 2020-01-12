#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Game {
public:
    GLFWwindow* window;
    glm::dvec2 mouse_pos;

    Game(GLFWwindow* window) : window(window) {}

    void init() {

    }

    void update() {
        int window_w, window_h;
        glfwGetFramebufferSize(window, &window_w, &window_h);

        glViewport(0, 0, window_w, window_h);
        glClearColor(0.25, 0.45, 0.75, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
};
