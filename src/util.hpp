#pragma once
#include <cmath>
#include <fstream>
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

static std::string file_read(std::string path) {
    /// http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
    if (path == "") return "";
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (f) {
        std::string contents;
        f.seekg(0, std::ios::end);  // read to the end of the file
        contents.resize(f.tellg()); // get position of the last character (size of file)

        f.seekg(0, std::ios::beg); // go to beginning
        f.read(&contents[0], contents.size());
        f.close(); // read whole file and close

        return contents;
    }
    throw std::runtime_error("Could not open file: " + path);
}

template <typename II>
static std::string join(II b, II e, std::string delim) {
    std::stringstream s;
    bool first = true;
    for (auto it = b; it != e; ++it) {
        if (first) { first = false; }
        else { s << delim; }
        s << *it;
    }
    return s.str();
}
