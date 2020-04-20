#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include <glad/glad.h>

static void check_program_errors(GLuint program_id) {
    GLint is_ok = 0;
    glGetProgramiv(program_id, GL_LINK_STATUS, &is_ok);
    if (!is_ok) {
        GLint max_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &max_length);

        // The maxLength includes the NULL character
        std::vector<GLchar> info_log(max_length);
        glGetProgramInfoLog(program_id, max_length, NULL, &info_log[0]);

        const std::string err = "GLSL linking error for program: ";
        throw std::runtime_error(err + "\n" + info_log.data());
    }
}
