#pragma once
#include <cstdlib>
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

#include "gfx/program.hpp"
#include "gfx/rendertexture.hpp"
#include "Box.hpp"
#include "Fluid.hpp"
#include "Quad.hpp"
#include "util.hpp"

class Game {
public:
    const float pi = glm::pi<float>();

    GLFWwindow* window;
    glm::vec2 mouse_pos = glm::vec2(0);
    glm::vec2 mouse_right_drag_start = glm::vec2(0);
    bool mouse_left_dragging = false;
    bool mouse_right_dragging = false;
    glm::vec3 old_world_mouse_pos = glm::vec3(0);

    const float camera_speed = 0.003;
    float camera_yaw = 0;
    float camera_pitch = 0;

    Fluid fluid;
    bool running = false;
    bool do_step = false;
    bool grid_visible = false;
    bool particles_visible = true;
    bool use_ssf = true;
    int grid_display_mode = 0;

    Box box;

    gfx::RenderTexture scene_texture;
    gfx::Program texture_copy_program;
    Quad quad;

    Game(GLFWwindow* window) : window(window) {}

    void init() {
        srand(time(0));
        fluid.init();
        texture_copy_program.vertex({"screen_quad.vs.glsl"}).fragment({"texture_copy.fs.glsl"}).compile();
    }

    void resize(uint w, uint h) {
        fluid.resize(w, h);
        scene_texture.set_texture_size(w, h);
    }

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

        const glm::vec4 viewport(0, 0, window_w, window_h);
        glViewport(0, 0, window_w, window_h);

        // projection matrix
        const float aspect_ratio = static_cast<double>(window_w) / window_h;
        const glm::mat4 projection = glm::perspective(glm::radians(30.f), aspect_ratio, 0.1f, 100.f);

        // view matrix (orbit camera) 
        glm::vec3 eye(0, 0, -6);
        // TODO: use quaternions
        eye = glm::rotate(eye, camera_yaw, glm::vec3(0, 1, 0));
        eye = glm::rotate(eye, camera_pitch, glm::cross(glm::vec3(0, 1, 0), eye));
        const glm::mat4 view = glm::lookAt(eye, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        const glm::vec3 look = -glm::xyz(glm::inverse(projection * view) * glm::vec4(0, 0, 1, 0));
        fluid.eye = eye;
        fluid.look = look;

        double mouse_pos_x, mouse_pos_y;
        glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
        glm::vec3 mouse_world = glm::unProject(glm::vec3(mouse_pos_x, window_h - mouse_pos_y, 0), view, projection, viewport);
        fluid.world_mouse_pos = mouse_world;
        if (mouse_left_dragging) {
            fluid.world_mouse_vel = ((fluid.world_mouse_pos + glm::normalize(fluid.world_mouse_pos - eye) * 6.f) - (old_world_mouse_pos + glm::normalize(old_world_mouse_pos - eye) * 6.f)) * 10.f;
        } else {
            fluid.world_mouse_vel = glm::vec3(0.f);
        }
        old_world_mouse_pos = fluid.world_mouse_pos;

        // simulation step
        if (running or do_step) {
            do_step = false;
            fluid.step();
            fluid.ssbo_barrier();
        }

        // clear screen
        glClearColor(0.16, 0.14, 0.10, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render the scene
        scene_texture.bind_framebuffer();
            // clear scene buffer
            glClearColor(0.16, 0.14, 0.10, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

            box.draw(projection, view, eye);
            if (!use_ssf and particles_visible)
                fluid.draw_particles(projection, view, viewport);
            if (grid_visible)
                fluid.draw_grid(projection, view, grid_display_mode);
        scene_texture.unbind_framebuffer();

        // copy scene buffer to screen
        texture_copy_program.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scene_texture.color_id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, scene_texture.depth_id);
        quad.draw();
        texture_copy_program.disuse();

        // render screen space fluid if applicable
        if (use_ssf and particles_visible)
            fluid.draw_particles_ssf(scene_texture, projection, view, viewport);

        fluid.draw_debug_lines(projection, view);
    }
};
