#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Particle.hpp"
#include "util.hpp"
#include "gfx/object.hpp"
#include "gfx/program.hpp"

struct Particles {
    const int num_circle_vertices = 8; // circle detail for particle rendering
    const int num_particles;

    gfx::Buffer ssbo{GL_SHADER_STORAGE_BUFFER}; // particle data storage
    gfx::Buffer circle_verts{GL_ARRAY_BUFFER};
    gfx::VAO vao;

    gfx::Program compute_program; // compute shader to operate on particles SSBO
    gfx::Program program; // program for particle rendering

    Particles(int num_particles) : num_particles(num_particles) {
        std::vector<Particle> initial;
        for (int i = 0; i < num_particles; ++i) {
            initial.emplace_back(Particle{
                glm::linearRand(glm::vec3(-1.0), glm::vec3(1.0)),
                glm::sphericalRand(0.001),
                glm::linearRand(glm::vec4(0.0), glm::vec4(1.0))
            });
        }
        ssbo.bind_base(0).set_data(initial);

        // circle vertices (for triangle fan)
        std::vector<glm::vec2> circle;
        for (int i = 0; i < num_circle_vertices; ++i) {
            const float f = static_cast<float>(i) / num_circle_vertices * glm::pi<float>() * 2.0;
            circle.emplace_back(glm::vec2(glm::sin(f), glm::cos(f)));
        }
        circle_verts.set_data(circle);

        vao.bind_attrib(circle_verts, 2, GL_FLOAT)
           .bind_attrib(ssbo, offsetof(Particle, pos), sizeof(Particle), 3, GL_FLOAT, gfx::INSTANCED)
           .bind_attrib(ssbo, offsetof(Particle, color), sizeof(Particle), 4, GL_FLOAT, gfx::INSTANCED);

        compute_program.compute({"particles.cs.glsl"}).compile();
        program.vertex({"particles.vs.glsl"}).fragment({"particles.fs.glsl"}).compile();
    }

    void dispatch_compute() {
        // https://www.khronos.org/opengl/wiki/Compute_Shader#Dispatch
        compute_program.use();
        glDispatchCompute(num_particles, 1, 1);
        compute_program.disuse();
    }

    void await_compute() {
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMemoryBarrier.xhtml
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void draw(const glm::mat4& projection, const glm::mat4& view) {
        program.use();
        glUniformMatrix4fv(program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
        vao.bind();
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, num_particles);
        vao.unbind();
        program.disuse();
    }
};
