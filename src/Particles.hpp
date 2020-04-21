#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Particle.hpp"
#include "util.hpp"
#include "gfx/object.hpp"

struct Particles {
    const int num_circle_vertices = 8; // circle detail for particle rendering
    const int num_particles;

    gfx::Buffer ssbo{GL_SHADER_STORAGE_BUFFER}; // particle data storage
    gfx::Buffer circle_verts{GL_ARRAY_BUFFER};
    gfx::VAO vao;

    GLuint compute_program; // compute shader to operate on particles SSBO
    GLuint program;         // program for particle rendering

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

        create_compute_program();
        create_program();
    }

    void create_compute_program() {
        constexpr char const* compute_src = R"(
            #version 430 core
            layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

            struct Particle {
                vec3 pos;
                float pad0;
                vec3 vel;
                float pad1;
                vec4 color;
            };

            layout(std430, binding=0) buffer ParticleBlock {
                Particle particle[];
            };

            void main() {
                uint index = gl_WorkGroupID.x;
                // particle[index].pos += particle[index].vel;
            }
        )";

        compute_program = glCreateProgram();
        GLuint cs_id = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(cs_id, 1, &compute_src, NULL);
        glCompileShader(cs_id);
        glAttachShader(compute_program, cs_id);
        glLinkProgram(compute_program);
        check_program_errors(compute_program);
    }

    void create_program() {
        constexpr char const* vert_src = R"(
            #version 430 core
            layout(location=0) in vec2 circle_offset;
            layout(location=1) in vec3 particle_pos;
            layout(location=2) in vec4 particle_color;
            out vec4 color;
            uniform mat4 projection;
            uniform mat4 view;

            void main() {
                const float radius = 0.025;
                vec3 particle_pos_world = (view * vec4(particle_pos, 1.0)).xyz;
                vec3 vertex_pos_world = particle_pos_world + vec3(circle_offset, 0) * radius * 0.5;
                gl_Position = projection * vec4(vertex_pos_world, 1.0);
                color = particle_color;
            }
        )";
        constexpr char const* frag_src = R"(
            #version 430 core
            in vec4 color;
            out vec4 frag_color;

            void main() {
                frag_color = color;
            }
        )";

        program = glCreateProgram();
        
        GLuint vert_id = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert_id, 1, &vert_src, NULL);
        glCompileShader(vert_id);
        glAttachShader(program, vert_id);

        GLuint frag_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag_id, 1, &frag_src, NULL);
        glCompileShader(frag_id);
        glAttachShader(program, frag_id);

        glLinkProgram(program);
        check_program_errors(program);
    }

    void dispatch_compute() {
        // https://www.khronos.org/opengl/wiki/Compute_Shader#Dispatch
        glUseProgram(compute_program);
        glDispatchCompute(num_particles, 1, 1);
        glUseProgram(0); // unbind
    }

    void await_compute() {
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMemoryBarrier.xhtml
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void draw(const glm::mat4& projection, const glm::mat4& view) {
        glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        vao.bind();
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, num_particles);
        vao.unbind();
        glUseProgram(0);
    }
};
