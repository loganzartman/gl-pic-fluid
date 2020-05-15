#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <functional>

#include <glad/glad.h>

#include "../util.hpp"

namespace gfx {
static std::string shader_prepend = "#version 430 core\n";
static std::string shader_root = "shader";

class Program {
public:
    Program() : name("[unnamed]") {}
    Program(std::string name) : name(name) {}
    ~Program() {
        if (id) { glDeleteProgram(id); }
        if (vertex_id) { glDeleteShader(vertex_id); }
        if (geometry_id) { glDeleteShader(geometry_id); }
        if (fragment_id) { glDeleteShader(fragment_id); }
        if (compute_id) { glDeleteShader(compute_id); }
    }

    Program& vertex(std::initializer_list<std::string> srcs) {
        compile_shader(srcs, vertex_id, GL_VERTEX_SHADER, "vertex");
        return *this;
    }

    Program& geometry(std::initializer_list<std::string> srcs) {
        compile_shader(srcs, geometry_id, GL_GEOMETRY_SHADER, "geometry");
        return *this;
    }

    Program& fragment(std::initializer_list<std::string> srcs) {
        compile_shader(srcs, fragment_id, GL_FRAGMENT_SHADER, "fragment");
        return *this;
    }

    Program& compute(std::initializer_list<std::string> srcs) {
        compile_shader(srcs, compute_id, GL_COMPUTE_SHADER, "compute");
        return *this;
    }

    void compile_shader(std::initializer_list<std::string> srcs, GLuint& dest, GLenum type, std::string type_name) {
        std::string src = read_sources(srcs);
        dest = glCreateShader(type);
        const char* src_str = src.c_str();
        glShaderSource(dest, 1, &src_str, NULL);
        glCompileShader(dest);
        try {
            check_shader_errors(dest);
        } catch (std::runtime_error& e) {
            throw std::runtime_error(type_name + " shader compilation error in files: " + join(srcs.begin(), srcs.end(), ", ") + "\n" + e.what());
        }
    }

    Program& compile() {
        id = glCreateProgram();

        // attach shaders
        if (compute_id) {
            glAttachShader(id, compute_id);
        }
        else {
            if (!vertex_id) { throw std::runtime_error("Compiling program without vertex shader loaded."); }
            if (!fragment_id) { throw std::runtime_error("Compiling program without fragment shader loaded."); }
            glAttachShader(id, vertex_id);
            if (geometry_id) { glAttachShader(id, geometry_id); }
            glAttachShader(id, fragment_id);
        }
        
        glLinkProgram(id);
        check_program_errors(id);
        return *this;
    }

    GLint uniform_loc(std::string uname) {
        GLint location = glGetUniformLocation(id, uname.c_str());
        // if (location < 0) { std::cerr << "Warning in " << name << ": invalid or unused uniform: " << uname << std::endl; }
        return location;
    }
    
    void use() {
        if (!id) { throw std::runtime_error("Trying to use program that is not compiled."); }
        glUseProgram(id); 
    }

    void validate() {
        glValidateProgram(id);
        check_validation(id);
    }

    void disuse() {
        glUseProgram(0);
    }

    std::string name;
    GLuint id = 0;
    GLuint vertex_id = 0;
    GLuint geometry_id = 0;
    GLuint fragment_id = 0;
    GLuint compute_id = 0;

private:
    /**
     * Read and concatenate shader sources from the shader_root
     */
    std::string read_sources(std::initializer_list<std::string> srcs) {
        std::stringstream result;
        result << shader_prepend;
        for (auto& src : srcs) {
            result << file_read(shader_root + "/" + src) << std::endl;
        }
        return result.str();
    }

    void check_validation(GLuint shader) {
        GLint is_ok = 0;
        glGetProgramiv(id, GL_VALIDATE_STATUS, &is_ok);
        if (!is_ok) {
			GLint max_length = 0;
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &max_length);

            // The maxLength includes the NULL character
            std::vector<GLchar> info_log(max_length);
            glGetProgramInfoLog(id, max_length, NULL, &info_log[0]);

            const std::string err = "GLSL validation error for program: ";
            throw std::runtime_error(err + name + "\n" + info_log.data());
		}
    }

    void check_shader_errors(GLuint shader) {
        GLint is_ok = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &is_ok);
        if (!is_ok) {
			GLint max_length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

            // The maxLength includes the NULL character
            std::vector<GLchar> info_log(max_length);
            glGetShaderInfoLog(shader, max_length, NULL, &info_log[0]);

            const std::string err = "GL Program Validation error: \n";
            throw std::runtime_error(err + info_log.data());
		}
    }

    void check_program_errors(GLuint shader) {
        GLint is_ok = 0;
        glGetProgramiv(id, GL_LINK_STATUS, &is_ok);
        if (!is_ok) {
			GLint max_length = 0;
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &max_length);

            // The maxLength includes the NULL character
            std::vector<GLchar> info_log(max_length);
            glGetProgramInfoLog(id, max_length, NULL, &info_log[0]);

            const std::string err = "GLSL linking error for program: ";
            throw std::runtime_error(err + name + "\n" + info_log.data());
		}
    }
};
}
