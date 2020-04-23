#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GridCell.hpp"
#include "Particle.hpp"
#include "util.hpp"
#include "gfx/object.hpp"
#include "gfx/program.hpp"

struct Fluid {
    const int num_circle_vertices = 8; // circle detail for particle rendering

    const int particle_density = 8;
    const glm::ivec3 grid_dimensions{64, 64, 64};
    const glm::vec3 bounds_min{-1, -1, -1};
    const glm::vec3 bounds_max{1, 1, 1};
    const glm::vec3 bounds_size = bounds_max - bounds_min;
    const glm::vec3 cell_size = bounds_size / glm::vec3(grid_dimensions);

    gfx::Buffer particle_ssbo{GL_SHADER_STORAGE_BUFFER}; // particle data storage
    gfx::Buffer grid_ssbo{GL_SHADER_STORAGE_BUFFER}; // grid data storage
    gfx::Buffer circle_verts{GL_ARRAY_BUFFER};
    gfx::VAO vao;

    gfx::Program ptg_program; // particle to grid transfer shader
    gfx::Program compute_program; // compute shader to operate on particles SSBO
    gfx::Program program; // program for particle rendering

    Fluid() {
        std::vector<GridCell> initial_grid;
        std::vector<Particle> initial_particles;
        for (int gz = 0; gz < grid_dimensions.z; ++gz) {
            for (int gy = 0; gy < grid_dimensions.y; ++gy) {
                for (int gx = 0; gx < grid_dimensions.x; ++gx) {
                    const glm::ivec3 gpos{gx, gy, gz};

                    if (gy < grid_dimensions.y * 2 / 3 && gx < grid_dimensions.x / 3) {
                        initial_grid.emplace_back(GridCell{
                            glm::vec3(0),
                            GRID_FLUID
                        });
                        const glm::vec3 cell_pos = bounds_min + bounds_size * glm::vec3(gpos) / glm::vec3(grid_dimensions);
                        initial_particles.emplace_back(Particle{
                            glm::linearRand(cell_pos, cell_pos + cell_size),
                            glm::vec3(0),
                            glm::vec4(0.12,0.57,0.89,0.25)
                        });
                    } else {
                        initial_grid.emplace_back(GridCell{
                            glm::vec3(0),
                            GRID_AIR
                        });
                    }
                }
            }
        }
        particle_ssbo.bind_base(0).set_data(initial_particles);
        grid_ssbo.bind_base(1).set_data(initial_grid);
        std::cerr << "Cell count: " << initial_grid.size() << std::endl;
        std::cerr << "Particle count: " << initial_particles.size() << std::endl;

        // circle vertices (for triangle fan)
        std::vector<glm::vec2> circle;
        for (int i = 0; i < num_circle_vertices; ++i) {
            const float f = static_cast<float>(i) / num_circle_vertices * glm::pi<float>() * 2.0;
            circle.emplace_back(glm::vec2(glm::sin(f), glm::cos(f)));
        }
        circle_verts.set_data(circle);

        vao.bind_attrib(circle_verts, 2, GL_FLOAT)
           .bind_attrib(particle_ssbo, offsetof(Particle, pos), sizeof(Particle), 3, GL_FLOAT, gfx::INSTANCED)
           .bind_attrib(particle_ssbo, offsetof(Particle, color), sizeof(Particle), 4, GL_FLOAT, gfx::INSTANCED);

        ptg_program.compute({"common.glsl", "particle_to_grid.cs.glsl"}).compile();
        compute_program.compute({"common.glsl", "particles.cs.glsl"}).compile();
        program.vertex({"particles.vs.glsl"}).fragment({"particles.fs.glsl"}).compile();
    }

    void particle_to_grid() {
        ptg_program.use();
        glUniform3iv(ptg_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        glUniform3fv(ptg_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(ptg_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glDispatchCompute(1, 1, 1);
        ptg_program.disuse();
    }

    void dispatch_compute() {
        // https://www.khronos.org/opengl/wiki/Compute_Shader#Dispatch
        compute_program.use();
        // glDispatchCompute(particle_ssbo.length(), 1, 1);
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
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, particle_ssbo.length());
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        vao.unbind();
        program.disuse();
    }
};
