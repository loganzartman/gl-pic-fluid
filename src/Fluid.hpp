#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include "GridCell.hpp"
#include "Particle.hpp"
#include "DebugLine.hpp"
#include "util.hpp"
#include "gfx/object.hpp"
#include "gfx/program.hpp"

struct Fluid {
    const int num_circle_vertices = 16; // circle detail for particle rendering

    const int particle_density = 8;
    const int grid_size = 24;
    const glm::ivec3 grid_dimensions{grid_size + 1, grid_size + 1, grid_size + 1};
    const glm::ivec3 grid_cell_dimensions{grid_size, grid_size, grid_size};
    const glm::vec3 bounds_min{-1, -1, -1};
    const glm::vec3 bounds_max{1, 1, 1};
    const glm::vec3 bounds_size = bounds_max - bounds_min;
    const glm::vec3 cell_size = bounds_size / glm::vec3(grid_cell_dimensions);
    const glm::vec3 gravity{0, -9.8, 0};

    gfx::Buffer particle_ssbo{GL_SHADER_STORAGE_BUFFER}; // particle data storage
    gfx::Buffer grid_ssbo{GL_SHADER_STORAGE_BUFFER}; // grid data storage
    gfx::Buffer circle_verts{GL_ARRAY_BUFFER};
    gfx::Buffer debug_lines_ssbo{GL_SHADER_STORAGE_BUFFER};
    gfx::VAO vao;
    gfx::VAO grid_vao;
    gfx::VAO debug_lines_vao; // used for drawing colored lines for debugging

    gfx::Program particle_advect_program; // compute shader to operate on particles SSBO
    gfx::Program body_forces_program; // compute shader to apply body forces on grid
    gfx::Program extrapolate_program; // extrapolate grid velocities by one cell
    gfx::Program set_vel_known_program;
    gfx::Program setup_grid_project_program; // compute A and RHS of pressure equation
    gfx::Program jacobi_iterate_program; // single jacobi iteration to solve for pressure gradient 
    gfx::Program pressure_to_guess_program; // copy pressure to pressure_guess for pressure solve
    gfx::Program pressure_update_program; // update velocities from pressure gradient
    gfx::Program grid_to_particle_program;

    gfx::Program program; // program for particle rendering
    gfx::Program grid_program;
    gfx::Program debug_lines_program;

    Fluid() {}

    void init() {
        init_ssbos();

        // graphics initialization
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
           .bind_attrib(grid_ssbo, offsetof(GridCell, type), sizeof(GridCell), 1, GL_INT, gfx::NOT_INSTANCED)
           .bind_attrib(grid_ssbo, offsetof(GridCell, rhs), sizeof(GridCell), 1, GL_FLOAT, gfx::NOT_INSTANCED)
           .bind_attrib(grid_ssbo, offsetof(GridCell, a_diag), sizeof(GridCell), 4, GL_FLOAT, gfx::NOT_INSTANCED)
           .bind_attrib(grid_ssbo, offsetof(GridCell, pressure), sizeof(GridCell), 1, GL_FLOAT, gfx::NOT_INSTANCED)
           .bind_attrib(grid_ssbo, offsetof(GridCell, vel_unknown), sizeof(GridCell), 1, GL_INT, gfx::NOT_INSTANCED);

        debug_lines_vao.bind_attrib(debug_lines_ssbo, offsetof(DebugLine, a), sizeof(DebugLine), 3, GL_FLOAT, gfx::NOT_INSTANCED)
            .bind_attrib(debug_lines_ssbo, offsetof(DebugLine, b), sizeof(DebugLine), 3, GL_FLOAT, gfx::NOT_INSTANCED)
            .bind_attrib(debug_lines_ssbo, offsetof(DebugLine, color), sizeof(DebugLine), 4, GL_FLOAT, gfx::NOT_INSTANCED);
        
        grid_to_particle_program.compute({"common.glsl", "grid_to_particle.cs.glsl"}).compile();
        extrapolate_program.compute({"common.glsl", "extrapolate.cs.glsl"}).compile();
        set_vel_known_program.compute({"common.glsl", "set_vel_known.cs.glsl"}).compile();
        body_forces_program.compute({"common.glsl", "enforce_boundary.cs.glsl", "body_forces.cs.glsl"}).compile();
        setup_grid_project_program.compute({"common.glsl", "setup_project.cs.glsl", "compute_divergence.cs.glsl", "build_a.cs.glsl"}).compile();
        jacobi_iterate_program.compute({"common.glsl", "jacobi_iterate.cs.glsl"}).compile();
        pressure_to_guess_program.compute({"common.glsl", "pressure_to_guess.cs.glsl"}).compile();
        pressure_update_program.compute({"common.glsl", "pressure_update.cs.glsl"}).compile();
        particle_advect_program.compute({"common.glsl", "particle_advect.cs.glsl"}).compile();
        program.vertex({"particles.vs.glsl"}).fragment({"lighting.glsl", "particles.fs.glsl"}).compile();
        grid_program.vertex({"common.glsl", "grid.vs.glsl"}).geometry({"common.glsl", "grid.gs.glsl"}).fragment({"grid.fs.glsl"}).compile();
        debug_lines_program.vertex({"debug_lines.vs.glsl"}).geometry({"debug_lines.gs.glsl"}).fragment({"debug_lines.fs.glsl"}).compile();
    }

    void init_ssbos() {
        std::vector<GridCell> initial_grid;
        std::vector<Particle> initial_particles;
        for (int gz = 0; gz < grid_dimensions.z; ++gz) {
            for (int gy = 0; gy < grid_dimensions.y; ++gy) {
                for (int gx = 0; gx < grid_dimensions.x; ++gx) {
                    const glm::ivec3 gpos{gx, gy, gz};
                    const glm::vec3 cell_pos = get_world_coord(gpos);

                    if (gx < grid_cell_dimensions.x / 2 and gy < grid_cell_dimensions.y * 2 / 3) {
                        initial_grid.emplace_back(GridCell{
                            cell_pos,
                            glm::vec3(0),
                            GRID_FLUID
                        });
                        
                        if (gx < grid_cell_dimensions.x and gy < grid_cell_dimensions.y and gz < grid_cell_dimensions.z) {
                            for (int i = 0; i < particle_density; ++i) {
                                const glm::vec3 particle_pos = glm::linearRand(cell_pos, cell_pos + cell_size);
                                initial_particles.emplace_back(Particle{
                                    particle_pos,
                                    glm::vec3(0),
                                    glm::vec4(0.32,0.57,0.79,1.0)
                                });
                            }
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
        // initial_particles.emplace_back(Particle{
        //     (bounds_min + bounds_max) / 2.f + cell_size / 3.f,
        //     glm::vec3(0.005, 0.01, 0.01),
        //     glm::vec4(1, 0, 1, 1)
        // });
        particle_ssbo.bind_base(0).set_data(initial_particles, GL_DYNAMIC_READ);
        grid_ssbo.bind_base(1).set_data(initial_grid, GL_DYNAMIC_COPY);
        std::cerr << "Cell count: " << initial_grid.size() << std::endl;
        std::cerr << "Particle count: " << initial_particles.size() << std::endl;

        std::vector<DebugLine> debug_lines;
        debug_lines.push_back(DebugLine({0, 0, 0}, {0.1, 0, 0}, {1, 0, 0, 1})); // x axis
        debug_lines.push_back(DebugLine({0, 0, 0}, {0, 0.1, 0}, {0, 1, 0, 1})); // y axis
        debug_lines.push_back(DebugLine({0, 0, 0}, {0, 0, 0.1}, {0, 0, 1, 1})); // z axis
        debug_lines_ssbo.bind_base(2).set_data(debug_lines); 

        std::cout << "Size of debug lines buffer " << debug_lines_ssbo.length() << " (" << debug_lines_ssbo.size() << " bytes)" << std::endl;
    }

    glm::ivec3 get_grid_coord(const glm::vec3& pos, const glm::ivec3& half_offset = glm::ivec3(0, 0, 0)) {
        return glm::floor((pos + glm::vec3(half_offset) * (cell_size / 2.f) - bounds_min) / bounds_size * glm::vec3(grid_cell_dimensions));
    }

    glm::vec3 get_world_coord(const glm::ivec3& grid_coord, const glm::ivec3& half_offset = glm::ivec3(0, 0, 0)) {
        return bounds_min + glm::vec3(grid_coord) * cell_size + glm::vec3(half_offset) * cell_size * 0.5f;
    }

    bool grid_in_bounds(const glm::ivec3& grid_coord) {
        return (grid_coord.x >= 0 and grid_coord.y >= 0 and grid_coord.z >= 0 and
                grid_coord.x < grid_dimensions.x and grid_coord.y < grid_dimensions.y and grid_coord.z < grid_dimensions.z);
    }

    int get_grid_index(const glm::ivec3& grid_coord) {
        const glm::ivec3 clamped_coord = glm::clamp(grid_coord, glm::ivec3(0), grid_dimensions - glm::ivec3(1));
        return clamped_coord.z * grid_dimensions.x * grid_dimensions.y + clamped_coord.y * grid_dimensions.x + clamped_coord.x;
    }

    void particle_to_grid() {
        const auto particles = particle_ssbo.map_buffer_readonly<Particle>();
        auto grid = grid_ssbo.map_buffer<GridCell>();

        // clear grid values
        for (int i = 0; i < grid_ssbo.length(); ++i) {
            grid[i].type = GRID_AIR;
            grid[i].vel = glm::vec3(0);
        }

        // particle to cell transfer
        std::unordered_map<int, glm::vec3> cell_weights;

        auto transfer_part = [&](const glm::ivec3& grid_coord, const glm::vec3& weights, const glm::vec3& value) {
            const int index = get_grid_index(grid_coord);
            const float weight = glm::compMul(weights);
            grid[index].vel += value * weight;
            grid[index].vel_unknown = 0;
            if (value.x != 0)
                cell_weights[index] += glm::vec3(weight, 0, 0);
            if (value.y != 0)
                cell_weights[index] += glm::vec3(0, weight, 0);
            if (value.z != 0)
                cell_weights[index] += glm::vec3(0, 0, weight);
        };
        
        auto transfer = [&](const glm::ivec3& base_coord, const glm::vec3& weights, const glm::vec3& value) {
            transfer_part(base_coord + glm::ivec3(0, 0, 0), {  weights.x,   weights.y,   weights.z}, value);
            transfer_part(base_coord + glm::ivec3(1, 0, 0), {1-weights.x,   weights.y,   weights.z}, value);
            transfer_part(base_coord + glm::ivec3(0, 1, 0), {  weights.x, 1-weights.y,   weights.z}, value);
            transfer_part(base_coord + glm::ivec3(0, 0, 1), {  weights.x,   weights.y, 1-weights.z}, value);
            transfer_part(base_coord + glm::ivec3(1, 1, 0), {1-weights.x, 1-weights.y,   weights.z}, value);
            transfer_part(base_coord + glm::ivec3(0, 1, 1), {  weights.x, 1-weights.y, 1-weights.z}, value);
            transfer_part(base_coord + glm::ivec3(1, 0, 1), {1-weights.x,   weights.y, 1-weights.z}, value);
            transfer_part(base_coord + glm::ivec3(1, 1, 1), {1-weights.x, 1-weights.y, 1-weights.z}, value);
        };

        for (int i = 0; i < particle_ssbo.length(); ++i) {
            const Particle& p = particles[i];
            const glm::ivec3 grid_coord_center = get_grid_coord(p.pos);
            const int center_index = get_grid_index(grid_coord_center);
            grid[center_index].type = GRID_FLUID;

            // u
            {
                const glm::ivec3 base_coord = get_grid_coord(p.pos, {-1, 0, 0});
                const glm::vec3 weights = (p.pos - get_world_coord(base_coord, {1, 0, 0})) / cell_size;
                transfer(base_coord, weights, {p.vel.x, 0, 0});
            }
            // v
            {
                const glm::ivec3 base_coord = get_grid_coord(p.pos, {0, -1, 0});
                const glm::vec3 weights = (p.pos - get_world_coord(base_coord, {0, 1, 0})) / cell_size;
                transfer(base_coord, weights, {0, p.vel.y, 0});
            }
            // w
            {
                const glm::ivec3 base_coord = get_grid_coord(p.pos, {0, 0, -1});
                const glm::vec3 weights = (p.pos - get_world_coord(base_coord, {0, 0, 1})) / cell_size;
                transfer(base_coord, weights, {0, 0, p.vel.z});
            }
        }

        // divide out weights
        for (int i = 0; i < grid_ssbo.length(); ++i) {
            // do this conditionally to avoid 0/0 NaNs
            if (grid[i].vel.x != 0)
                grid[i].vel.x /= cell_weights[i].x;
            if (grid[i].vel.y != 0)
                grid[i].vel.y /= cell_weights[i].y;
            if (grid[i].vel.z != 0)
                grid[i].vel.z /= cell_weights[i].z;
        }
    }

    void extrapolate() {
        extrapolate_program.use();
        glUniform3fv(extrapolate_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(extrapolate_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(extrapolate_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        extrapolate_program.validate();
    
        set_vel_known_program.use();
        glUniform3fv(set_vel_known_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(set_vel_known_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(set_vel_known_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        set_vel_known_program.validate();

        for (int i = 0; i < glm::compMax(grid_dimensions) * 2; ++i) {
            ssbo_barrier();
            extrapolate_program.use();
            glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);

            ssbo_barrier();
            set_vel_known_program.use();
            glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
        }
    }

    void apply_body_forces(float dt) {
        // also enforces boundary condition
        ssbo_barrier();
        body_forces_program.use();
        const glm::vec3 body_force = gravity; // TODO: other forces?
        glUniform1f(body_forces_program.uniform_loc("dt"), dt);
        glUniform3fv(body_forces_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(body_forces_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(body_forces_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        glUniform3fv(body_forces_program.uniform_loc("body_force"), 1, glm::value_ptr(body_force));
        body_forces_program.validate();
        glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
        body_forces_program.disuse();
    }

    void setup_grid_project(float dt) {
        ssbo_barrier();
        setup_grid_project_program.use();
        glUniform1f(setup_grid_project_program.uniform_loc("dt"), dt);
        glUniform3fv(setup_grid_project_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(setup_grid_project_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(setup_grid_project_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        setup_grid_project_program.validate();
        glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
        setup_grid_project_program.disuse();
    }

    void jacobi_solve(float dt) {
        const int iters = 40;

        jacobi_iterate_program.use();
        glUniform1f(setup_grid_project_program.uniform_loc("dt"), dt);
        glUniform3fv(jacobi_iterate_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(jacobi_iterate_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(jacobi_iterate_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        jacobi_iterate_program.validate();

        pressure_to_guess_program.use();
        glUniform3fv(pressure_to_guess_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(pressure_to_guess_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(pressure_to_guess_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        pressure_to_guess_program.validate();

        for (int i = 0; i < iters; ++i) {
            ssbo_barrier();
            jacobi_iterate_program.use();
            glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);

            ssbo_barrier();
            pressure_to_guess_program.use();
            glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
        }
    }

    void pressure_update(float dt) {
        ssbo_barrier();
        pressure_update_program.use();
        glUniform1f(pressure_update_program.uniform_loc("dt"), dt);
        glUniform3fv(pressure_update_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(pressure_update_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(pressure_update_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        pressure_update_program.validate();
        glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
        pressure_update_program.disuse();
    }

    void grid_project(float dt) {
        setup_grid_project(dt);
        jacobi_solve(dt);
        pressure_update(dt);
    }

    void grid_to_particle() {
        ssbo_barrier();
        grid_to_particle_program.use();
        glUniform3fv(grid_to_particle_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(grid_to_particle_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(grid_to_particle_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        glUniform1f(grid_to_particle_program.uniform_loc("pic_flip_blend"), 0.);
        grid_to_particle_program.validate();
        glDispatchCompute(particle_ssbo.length(), 1, 1);
        grid_to_particle_program.disuse();
    }

    void particle_advect(float dt) {
        ssbo_barrier();
        particle_advect_program.use();
        glUniform1f(particle_advect_program.uniform_loc("dt"), dt);
        glUniform3fv(particle_advect_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(particle_advect_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glDispatchCompute(particle_ssbo.length(), 1, 1);
        particle_advect_program.disuse();
    }

    void ssbo_barrier() {
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMemoryBarrier.xhtml
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    void step() {
        const float dt = 0.01;
        particle_to_grid();
        extrapolate();
        apply_body_forces(dt);
        grid_project(dt);
        grid_to_particle();
        particle_advect(dt);
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

    void draw_grid(const glm::mat4& projection, const glm::mat4& view, int display_mode) {
        grid_program.use();
        glUniform3fv(grid_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(grid_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(grid_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        glUniformMatrix4fv(grid_program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(grid_program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform1i(grid_program.uniform_loc("display_mode"), display_mode);
        grid_vao.bind();
        glPointSize(16.0);
        glDrawArrays(GL_POINTS, 0, grid_ssbo.length());
        grid_vao.unbind();
        grid_program.disuse();
    }

    void draw_debug_lines(const glm::mat4& projection, const glm::mat4& view) {
        debug_lines_program.use();
        glUniformMatrix4fv(debug_lines_program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(debug_lines_program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
        debug_lines_vao.bind();
        glLineWidth(4.0);
        glDrawArrays(GL_POINTS, 0, debug_lines_ssbo.length());
        debug_lines_vao.unbind();
        debug_lines_program.disuse();
    }
};
