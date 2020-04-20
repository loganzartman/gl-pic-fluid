#pragma once
#include <array>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vec_swizzle.hpp>

#include "Box.hpp"
#include "Particles.hpp"
#include "util.hpp"

class Game {
public:
    const float pi = glm::pi<float>();

    GLFWwindow* window;
    glm::vec2 mouse_pos = glm::vec2(0);
    glm::vec2 mouse_right_drag_start = glm::vec2(0);
    bool mouse_right_dragging = false;

    const float camera_speed = 0.003;
    float camera_yaw;
    float camera_pitch;

    Particles particles = Particles(500);
    Box box;    

    Game(GLFWwindow* window) : window(window) {}

    void init() {}

    void update_camera() {
        if (mouse_right_dragging) {
            const glm::vec2 dx = (mouse_pos - mouse_right_drag_start) * camera_speed;
            camera_yaw -= dx.x;
            camera_pitch -= dx.y;

            const float pitch_limit = pi/2-0.001;
            camera_pitch = glm::clamp(camera_pitch, -pitch_limit, pitch_limit);
            mouse_right_drag_start = mouse_pos;
        }
    }

    void update() {
        const double t = glfwGetTime();
        int window_w, window_h;
        glfwGetFramebufferSize(window, &window_w, &window_h);

        update_camera();
        particles.dispatch_compute();

        glViewport(0, 0, window_w, window_h);

        // projection matrix
        const float aspect_ratio = static_cast<double>(window_w) / window_h;
        const glm::mat4 projection = glm::perspective(glm::radians(30.f), aspect_ratio, 0.1f, 100.f);

        // view matrix (orbit camera) 
        glm::vec3 eye(0, 0, 5);
        eye = glm::rotate(eye, camera_yaw, glm::vec3(0, 1, 0));
        eye = glm::rotate(eye, camera_pitch, glm::cross(glm::vec3(0, 1, 0), eye));
        const glm::mat4 view = glm::lookAt(eye, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        // clear screen
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.16, 0.14, 0.10, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        particles.await_compute();
        particles.draw(projection, view);
        box.draw(projection, view);
    }
};
