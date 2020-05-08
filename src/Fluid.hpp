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
    const int num_circle_vertices = 16; // circle detail for particle rendering

    const int particle_density = 8;
    const int grid_size = 24;
    const glm::ivec3 grid_dimensions{grid_size, grid_size, grid_size};
    const glm::vec3 bounds_min{-1, -1, -1};
    const glm::vec3 bounds_max{1, 1, 1};
    const glm::vec3 bounds_size = bounds_max - bounds_min;
    const glm::vec3 cell_size = bounds_size / glm::vec3(grid_dimensions);

    gfx::Buffer particle_ssbo{GL_SHADER_STORAGE_BUFFER}; // particle data storage
    gfx::Buffer grid_ssbo{GL_SHADER_STORAGE_BUFFER}; // grid data storage
    gfx::Buffer circle_verts{GL_ARRAY_BUFFER};
    gfx::VAO vao;
    gfx::VAO grid_vao;

    gfx::Program particle_update_program; // compute shader to operate on particles SSBO
    gfx::Program grid_to_particle_program;
    gfx::Program program; // program for particle rendering
    gfx::Program grid_program;

    Fluid() {
        std::vector<GridCell> initial_grid;
        std::vector<Particle> initial_particles;
        for (int gz = 0; gz < grid_dimensions.z; ++gz) {
            for (int gy = 0; gy < grid_dimensions.y; ++gy) {
                for (int gx = 0; gx < grid_dimensions.x; ++gx) {
                    const glm::ivec3 gpos{gx, gy, gz};
                    const glm::vec3 cell_pos = bounds_min + bounds_size * glm::vec3(gpos) / glm::vec3(grid_dimensions);

                    if (gy < grid_dimensions.y * 2 / 3 && gx < grid_dimensions.x / 3) {
                        initial_grid.emplace_back(GridCell{
                            cell_pos,
                            glm::vec3(0),
                            GRID_FLUID
                        });
                        for (int i = 0; i < particle_density; ++i) {
                            initial_particles.emplace_back(Particle{
                                glm::linearRand(cell_pos, cell_pos + cell_size),
                                glm::ballRand(0.001),
                                glm::vec4(0.32,0.57,0.79,1.0)
                            });
                        }
                    } else {
                        initial_grid.emplace_back(GridCell{
                            cell_pos,
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
           .bind_attrib(particle_ssbo, offsetof(Particle, vel), sizeof(Particle), 3, GL_FLOAT, gfx::INSTANCED)
           .bind_attrib(particle_ssbo, offsetof(Particle, color), sizeof(Particle), 4, GL_FLOAT, gfx::INSTANCED);
        
        grid_vao.bind_attrib(grid_ssbo, offsetof(GridCell, pos), sizeof(GridCell), 3, GL_FLOAT, gfx::NOT_INSTANCED)
                .bind_attrib(grid_ssbo, offsetof(GridCell, vel), sizeof(GridCell), 3, GL_FLOAT, gfx::NOT_INSTANCED)
                .bind_attrib(grid_ssbo, offsetof(GridCell, marker), sizeof(GridCell), 1, GL_INT, gfx::NOT_INSTANCED);

        grid_to_particle_program.compute({"common.glsl", "grid_to_particle.cs.glsl"}).compile();
        particle_update_program.compute({"common.glsl", "particle_update.cs.glsl"}).compile();
        program.vertex({"particles.vs.glsl"}).fragment({"lighting.glsl", "particles.fs.glsl"}).compile();
        grid_program.vertex({"common.glsl", "grid.vs.glsl"}).fragment({"grid.fs.glsl"}).compile();
    }

    void particle_to_grid() {
        const auto particles = particle_ssbo.map_buffer<Particle>();
        auto grid = grid_ssbo.map_buffer<GridCell>();

        auto in_bounds = [&](const glm::ivec3& x){
            return (x.x >= 0 and x.y >= 0 and x.z >= 0 and
                    x.x < grid_dimensions.x and x.y < grid_dimensions.y and x.z < grid_dimensions.z);
        };
        
        for (int i = 0; i < grid_ssbo.length(); ++i) {
            grid[i].marker = GRID_AIR;
            grid[i].vel = glm::vec3(0);
        }

        std::unordered_map<int, uint> particle_counts;
        for (int i = 0; i < particle_ssbo.length(); ++i) {
            const Particle& p = particles[i];
            const glm::ivec3 grid_coord = glm::floor((p.pos - bounds_min) / bounds_size * glm::vec3(grid_dimensions));
            if (!in_bounds(grid_coord)) {
                continue;
            }
            const int index = grid_coord.z * grid_dimensions.x * grid_dimensions.y + 
                grid_coord.y * grid_dimensions.x + grid_coord.x;
            grid[index].marker = GRID_FLUID;
            grid[index].vel += p.vel;
            particle_counts[index] += 1;
        }

        for (int i = 0; i < grid_ssbo.length(); ++i) {
            grid[i].vel *= 1.0 / particle_counts[i];
        }
    }

    void grid_to_particle() {
        ssbo_barrier();
        grid_to_particle_program.use();
        glUniform3fv(grid_to_particle_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(grid_to_particle_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(grid_to_particle_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        grid_to_particle_program.validate();
        glDispatchCompute(particle_ssbo.length(), 1, 1);
        grid_to_particle_program.disuse();
    }

    void particle_update() {
        ssbo_barrier();
        particle_update_program.use();
        glDispatchCompute(particle_ssbo.length(), 1, 1);
        particle_update_program.disuse();
    }

    void ssbo_barrier() {
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMemoryBarrier.xhtml
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void draw_particles(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& viewport) {
        program.use();
        glUniformMatrix4fv(program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform4fv(program.uniform_loc("viewport"), 1, glm::value_ptr(viewport));
        const glm::vec3 look = glm::xyz(glm::inverse(projection * view) * glm::vec4(0, 0, 1, 0));
        glUniform3fv(program.uniform_loc("look"), 1, glm::value_ptr(look));
        vao.bind();
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, particle_ssbo.length());
        vao.unbind();
        program.disuse();
    }

    void draw_grid(const glm::mat4& projection, const glm::mat4& view) {
        grid_program.use();
        glUniformMatrix4fv(grid_program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(grid_program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
        grid_vao.bind();
        glPointSize(5.0);
        glDrawArrays(GL_POINTS, 0, grid_ssbo.length());
        grid_vao.unbind();
        grid_program.disuse();
    }
};
