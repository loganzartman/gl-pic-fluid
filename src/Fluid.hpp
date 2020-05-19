#pragma once
#include <vector>
#include <stdexcept>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Sparse>
#include "GridCell.hpp"
#include "Particle.hpp"
#include "DebugLine.hpp"
#include "P2GTransfer.hpp"
#include "SSFBufferElement.hpp"
#include "SSFRenderTexture.hpp"
#include "Quad.hpp"
#include "util.hpp"
#include "gfx/object.hpp"
#include "gfx/program.hpp"
#include "gfx/rendertexture.hpp"

struct Fluid {
    const int num_circle_vertices = 16; // circle detail for particle rendering

    const int particle_density = 16;
    const int grid_size = 24;
    const glm::ivec3 grid_dimensions{grid_size + 1, grid_size + 1, grid_size + 1};
    const glm::ivec3 grid_cell_dimensions{grid_size, grid_size, grid_size};
    const glm::vec3 bounds_min{-1, -1, -1};
    const glm::vec3 bounds_max{1, 1, 1};
    const glm::vec3 bounds_size = bounds_max - bounds_min;
    const glm::vec3 cell_size = bounds_size / glm::vec3(grid_cell_dimensions);
    const glm::vec3 gravity{0, -9.8, 0};
    glm::vec3 world_mouse_pos{0, -0.9, 0};
    glm::vec3 world_mouse_vel{0, 0, 0};
    glm::vec3 look{0, 0, 1};
    glm::vec3 eye{0, 0, 0};
    glm::ivec2 resolution{0, 0};

    gfx::Buffer viewport_rect{GL_ARRAY_BUFFER};
    gfx::Buffer particle_ssbo{GL_SHADER_STORAGE_BUFFER}; // particle data storage
    gfx::Buffer grid_ssbo{GL_SHADER_STORAGE_BUFFER}; // grid data storage
    gfx::Buffer transfer_ssbo{GL_SHADER_STORAGE_BUFFER}; // p2g transfer storage buffer
    gfx::Buffer circle_verts{GL_ARRAY_BUFFER};
    gfx::Buffer debug_lines_ssbo{GL_SHADER_STORAGE_BUFFER};
    gfx::VAO vao;
    gfx::VAO grid_vao;
    gfx::VAO debug_lines_vao; // used for drawing colored lines for debugging
    gfx::VAO screen_quad_vao; // fullscreen quad vertices

    gfx::Program reset_grid_program; // clear grid state
    gfx::Program p2g_accumulate_program; // accumulate new grid velocities from particles
    gfx::Program p2g_apply_program; // copy new grid velocities to grid data
    gfx::Program particle_advect_program; // compute shader to operate on particles SSBO
    gfx::Program body_forces_program; // compute shader to apply body forces on grid
    gfx::Program extrapolate_program; // extrapolate grid velocities by one cell
    gfx::Program set_vel_known_program;
    gfx::Program setup_grid_project_program; // compute A and RHS of pressure equation
    gfx::Program jacobi_iterate_program; // single jacobi iteration to solve for pressure gradient 
    gfx::Program pressure_to_guess_program; // copy pressure to pressure_guess for pressure solve
    gfx::Program pressure_update_program; // update velocities from pressure gradient
    gfx::Program grid_to_particle_program; // transfer grid velocities to particles

    gfx::Program program; // program for particle rendering
    gfx::Program grid_program;
    gfx::Program debug_lines_program;

    // screen space fluid rendering
    gfx::Program ssf_spheres_program; // SSF sphere rendering
    gfx::Program ssf_smooth_program; // SSF smoothing
    gfx::Program ssf_shade_program; // SSF shading

    gfx::RenderTexture scene_texture; // stores rendered things that are not fluid
    SSFRenderTexture ssf_a_texture; // stores sphere color/depth data, and world space position
    SSFRenderTexture ssf_b_texture; // it's double buffered for read/write

    Quad quad;

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
        
        
        reset_grid_program.compute({"common.glsl", "reset_grid.cs.glsl"}).compile();
        p2g_accumulate_program.compute({"common.glsl", "p2g_accumulate.cs.glsl"}).compile();
        p2g_apply_program.compute({"common.glsl", "p2g_apply.cs.glsl"}).compile();
        grid_to_particle_program.compute({"common.glsl", "grid_to_particle.cs.glsl"}).compile();
        extrapolate_program.compute({"common.glsl", "extrapolate.cs.glsl"}).compile();
        set_vel_known_program.compute({"common.glsl", "set_vel_known.cs.glsl"}).compile();
        body_forces_program.compute({"common.glsl", "enforce_boundary.cs.glsl", "body_forces.cs.glsl"}).compile();
        setup_grid_project_program.compute({"common.glsl", "setup_project.cs.glsl", "compute_divergence.cs.glsl", "build_a.cs.glsl"}).compile();
        jacobi_iterate_program.compute({"common.glsl", "jacobi_iterate.cs.glsl"}).compile();
        pressure_to_guess_program.compute({"common.glsl", "pressure_to_guess.cs.glsl"}).compile();
        pressure_update_program.compute({"common.glsl", "pressure_update.cs.glsl"}).compile();
        particle_advect_program.compute({"common.glsl", "rand.glsl", "particle_advect.cs.glsl"}).compile();
        
        program.vertex({"particles.vs.glsl"}).fragment({"lighting.glsl", "particles.fs.glsl"}).compile();
        grid_program.vertex({"common.glsl", "grid.vs.glsl"}).geometry({"common.glsl", "grid.gs.glsl"}).fragment({"grid.fs.glsl"}).compile();
        debug_lines_program.vertex({"debug_lines.vs.glsl"}).geometry({"debug_lines.gs.glsl"}).fragment({"debug_lines.fs.glsl"}).compile();

        ssf_spheres_program.vertex({"particles.vs.glsl"}).fragment({"common.glsl", "ssf_spheres.fs.glsl"}).compile();
        ssf_smooth_program.vertex({"screen_quad.vs.glsl"}).fragment({"ssf_smooth.fs.glsl"}).compile();
        ssf_shade_program.vertex({"screen_quad.vs.glsl"}).fragment({"lighting.glsl", "ssf_shade.fs.glsl"}).compile();
    }

    void init_ssbos() {
        std::vector<GridCell> initial_grid;
        std::vector<Particle> initial_particles;
        std::vector<P2GTransfer> initial_transfer;
        for (int gz = 0; gz < grid_dimensions.z; ++gz) {
            for (int gy = 0; gy < grid_dimensions.y; ++gy) {
                for (int gx = 0; gx < grid_dimensions.x; ++gx) {
                    const glm::ivec3 gpos{gx, gy, gz};
                    const glm::vec3 cell_pos = get_world_coord(gpos);

                    initial_transfer.emplace_back(P2GTransfer());

                    const glm::ivec3& d = grid_cell_dimensions;
                    if (gx < d.x / 2) {
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
        particle_ssbo.bind_base(0).set_data(initial_particles, GL_DYNAMIC_COPY);
        grid_ssbo.bind_base(1).set_data(initial_grid, GL_DYNAMIC_COPY);
        std::cerr << "Cell count: " << initial_grid.size() << std::endl;
        std::cerr << "Particle count: " << initial_particles.size() << std::endl;

        std::vector<DebugLine> debug_lines;
        debug_lines.push_back(DebugLine({0, 0, 0}, {0.1, 0, 0}, {1, 0, 0, 1})); // x axis
        debug_lines.push_back(DebugLine({0, 0, 0}, {0, 0.1, 0}, {0, 1, 0, 1})); // y axis
        debug_lines.push_back(DebugLine({0, 0, 0}, {0, 0, 0.1}, {0, 0, 1, 1})); // z axis
        debug_lines_ssbo.bind_base(2).set_data(debug_lines); 

        transfer_ssbo.bind_base(3).set_data(initial_transfer, GL_DYNAMIC_COPY);

        std::cout << "Size of debug lines buffer " << debug_lines_ssbo.length() << " (" << debug_lines_ssbo.size() << " bytes)" << std::endl;
    }

    void resize(uint w, uint h) {
        scene_texture.set_texture_size(w, h);
        ssf_a_texture.set_texture_size(w, h);
        ssf_b_texture.set_texture_size(w, h);
        resolution.x = w;
        resolution.y = h;
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

    void set_common_uniforms(gfx::Program& program) {
        glUniform3fv(program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        glUniform2iv(program.uniform_loc("resolution"), 1, glm::value_ptr(resolution));
    }

    void reset_grid() {
        ssbo_barrier();
        reset_grid_program.use();
        set_common_uniforms(reset_grid_program);
        reset_grid_program.validate();
        glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
        reset_grid_program.disuse();
    }

    void particle_to_grid_cpu() {
        const auto particles = particle_ssbo.map_buffer_readonly<Particle>();
        auto grid = grid_ssbo.map_buffer<GridCell>();

        // clear grid values
        for (int x = 0; x < grid_dimensions.x; ++x) {
            for (int y = 0; y < grid_dimensions.y; ++y) {
                for (int z = 0; z < grid_dimensions.z; ++z) {
                    const int i = get_grid_index({x, y, z});
                    grid[i].type = (x < grid_cell_dimensions.x and y < grid_cell_dimensions.y and z < grid_cell_dimensions.z) ? GRID_AIR : GRID_SOLID;
                    grid[i].vel = glm::vec3(0);
                }
            }
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

    void particle_to_grid() {
        constexpr static int group_size = 1024;
        reset_grid();

        p2g_accumulate_program.use();
        set_common_uniforms(p2g_accumulate_program);

        // accumulate
        for (int i = 0; i < particle_ssbo.length() / group_size + 1; ++i) {
            glUniform1i(p2g_accumulate_program.uniform_loc("start_index"), i * group_size);
            p2g_accumulate_program.validate();
            ssbo_barrier();
            glDispatchCompute(1, 1, 1);
        } 

        // copy transfer accumulators to grid velocities
        ssbo_barrier();
        p2g_apply_program.use();
        set_common_uniforms(p2g_apply_program);
        glDispatchCompute(grid_dimensions.x, grid_dimensions.y, grid_dimensions.z);
        p2g_apply_program.disuse();
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

    void pressure_solve() {
        const int iters = 40;

        jacobi_iterate_program.use();
        set_common_uniforms(jacobi_iterate_program);
        jacobi_iterate_program.validate();

        pressure_to_guess_program.use();
        set_common_uniforms(pressure_to_guess_program);
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

    void pressure_solve_eigen() {
        ssbo_barrier();
        auto grid = grid_ssbo.map_buffer<GridCell>();
        const glm::ivec3& dim = grid_cell_dimensions;
        const int n = glm::compMul(dim);
        auto in_bounds = [&](const glm::ivec3& c) {
            return c.x >= 0 and c.y >= 0 and c.z >= 0 and c.x < dim.x and c.y < dim.y and c.z < dim.z;
        };
        auto index = [&](const glm::ivec3& c){
            return c.x * (dim.x * dim.y) + c.y * dim.x + c.z;
        };
        
        Eigen::SparseMatrix<float> A;
        Eigen::VectorXf b(n);

        std::vector<Eigen::Triplet<float>> triplets;

        auto add_neighbor = [&](const glm::ivec3& c, int dx, int dy, int dz){
            const glm::ivec3 offset = glm::ivec3(dx, dy, dz);
            if (in_bounds(c + offset)) {
                int i = get_grid_index(c);
                int io = get_grid_index(c + offset);
                float f = 0;
                if (dx < 0)
                    f = grid[io].a_x;
                else if (dx > 0)
                    f = grid[i].a_x;
                else if (dy < 0)
                    f = grid[io].a_y;
                else if (dy > 0)
                    f = grid[i].a_y;
                else if (dz < 0)
                    f = grid[io].a_z;
                else if (dz > 0)
                    f = grid[i].a_z;
                else
                    f = grid[i].a_diag;
                triplets.emplace_back(index(c), index(c + offset), f);
            }
        };

        for (int x = 0; x < dim.x; ++x) {
            for (int y = 0; y < dim.y; ++y) {
                for (int z = 0; z < dim.z; ++z) {
                    const glm::ivec3 coord = glm::ivec3(x,y,z);
                    add_neighbor(coord, -1, 0, 0);
                    add_neighbor(coord, 0, -1, 0);
                    add_neighbor(coord, 0, 0, -1);
                    add_neighbor(coord, 1, 0, 0);
                    add_neighbor(coord, 0, 1, 0);
                    add_neighbor(coord, 0, 0, 1);
                    add_neighbor(coord, 0, 0, 0);
                    b(index(coord)) = grid[get_grid_index(coord)].rhs;
                }
            }
        }

        A.resize(n, n);
        A.setFromTriplets(triplets.begin(), triplets.end());

        Eigen::ConjugateGradient<Eigen::SparseMatrix<float>, 1, Eigen::DiagonalPreconditioner<float>> solver;
        solver.compute(A);
        Eigen::VectorXf p = solver.solve(b);
        if (solver.info() != Eigen::Success) {
            throw std::runtime_error("Didn't converge");
        }

        for (int x = 0; x < dim.x; ++x) {
            for (int y = 0; y < dim.y; ++y) {
                for (int z = 0; z < dim.z; ++z) {
                    const glm::ivec3 coord = glm::ivec3(x, y, z);
                    grid[get_grid_index(coord)].pressure = p(index(coord));
                }
            }
        }
    }

    void grid_to_particle() {
        ssbo_barrier();
        grid_to_particle_program.use();
        glUniform3fv(grid_to_particle_program.uniform_loc("bounds_min"), 1, glm::value_ptr(bounds_min));
        glUniform3fv(grid_to_particle_program.uniform_loc("bounds_max"), 1, glm::value_ptr(bounds_max));
        glUniform3iv(grid_to_particle_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        glUniform1f(grid_to_particle_program.uniform_loc("pic_flip_blend"), 0.9);
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
        glUniform3iv(particle_advect_program.uniform_loc("grid_dim"), 1, glm::value_ptr(grid_dimensions));
        glUniform3fv(particle_advect_program.uniform_loc("eye"), 1, glm::value_ptr(eye));
        glUniform3fv(particle_advect_program.uniform_loc("mouse_pos"), 1, glm::value_ptr(world_mouse_pos));
        glUniform3fv(particle_advect_program.uniform_loc("mouse_vel"), 1, glm::value_ptr(world_mouse_vel));
        glDispatchCompute(particle_ssbo.length(), 1, 1);
        particle_advect_program.disuse();
    }

    void ssbo_barrier() {
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMemoryBarrier.xhtml
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    void step() {
        const float dt = 0.02;
        particle_to_grid();
        // extrapolate();
        apply_body_forces(dt);
        setup_grid_project(dt);
        pressure_solve();
        pressure_update(dt);
        grid_to_particle();
        particle_advect(dt);
    }

    void draw_particles(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& viewport) {
        program.use();
        glUniformMatrix4fv(program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform4fv(program.uniform_loc("viewport"), 1, glm::value_ptr(viewport));
        glUniform3fv(program.uniform_loc("look"), 1, glm::value_ptr(look));
        vao.bind();
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, particle_ssbo.length());
        vao.unbind();
        program.disuse();
    }

    void draw_particles_ssf(const gfx::RenderTexture& scene_texture, const glm::mat4& projection, const glm::mat4& view, const glm::vec4& viewport) {
        constexpr static GLenum ssf_draw_buffers[]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

        // render spheres and position data
        ssf_spheres_program.use();
            set_common_uniforms(ssf_spheres_program);
            glUniformMatrix4fv(ssf_spheres_program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(ssf_spheres_program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniform4fv(ssf_spheres_program.uniform_loc("viewport"), 1, glm::value_ptr(viewport));
            glUniform3fv(ssf_spheres_program.uniform_loc("look"), 1, glm::value_ptr(look));

            vao.bind();
            ssf_a_texture.bind_framebuffer();

            // clear buffers
            glDrawBuffers(2, ssf_draw_buffers);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // color pass
            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_ONE, GL_ONE);
            glUniform1i(ssf_spheres_program.uniform_loc("pass"), 0);
            constexpr static GLenum first_pass_buffers[]{GL_COLOR_ATTACHMENT0, GL_NONE};
            glDrawBuffers(2, first_pass_buffers);
            glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, particle_ssbo.length());

            // sphere position pass
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glUniform1i(ssf_spheres_program.uniform_loc("pass"), 1);
            constexpr static GLenum second_pass_buffers[]{GL_NONE, GL_COLOR_ATTACHMENT1};
            glDrawBuffers(2, second_pass_buffers);
            glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, particle_ssbo.length());
            
            ssf_a_texture.unbind_framebuffer();
            vao.unbind();
        ssf_spheres_program.disuse();

        // smooth sphere depths, do color thing
        glDisable(GL_BLEND);
        ssf_smooth_program.use();
            glUniform2iv(ssf_smooth_program.uniform_loc("resolution"), 1, glm::value_ptr(resolution));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssf_a_texture.color1_id);
            ssf_b_texture.bind_framebuffer();
            glDrawBuffers(2, ssf_draw_buffers);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            quad.draw();
            ssf_b_texture.unbind_framebuffer();
            glBindTexture(GL_TEXTURE_2D, 0);
        ssf_smooth_program.disuse();

        // shade fluid
        // glDisable(GL_DEPTH_TEST);
        ssf_shade_program.use();
            glUniform2iv(ssf_shade_program.uniform_loc("resolution"), 1, glm::value_ptr(resolution));
            glUniformMatrix4fv(ssf_shade_program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(ssf_shade_program.uniform_loc("inv_view"), 1, GL_FALSE, glm::value_ptr(glm::inverse(view)));
            glUniform3fv(ssf_shade_program.uniform_loc("look"), 1, glm::value_ptr(look));
            glUniform3fv(ssf_shade_program.uniform_loc("eye"), 1, glm::value_ptr(eye));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssf_a_texture.color_id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, ssf_a_texture.depth_id);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, ssf_b_texture.color1_id); // sphere position data
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, scene_texture.color_id);
            quad.draw();
            glBindTexture(GL_TEXTURE_2D, 0);
        ssf_shade_program.disuse();
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
