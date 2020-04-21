#pragma once
#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <glad/glad.h>

namespace gfx {
static constexpr GLuint NOT_INSTANCED = 0;
static constexpr GLuint INSTANCED = 1;

class Buffer {
    int _size = 0, _length = 0;
public:
    const GLuint target;
    GLuint id;
    
    Buffer(GLuint target) : target(target) {
        glGenBuffers(1, &id);
    }

    ~Buffer() {
        glDeleteBuffers(1, &id);
    }

    void bind() {
        glBindBuffer(target, id);
    }
    
    void unbind() {
        glBindBuffer(target, 0);
    }

    int size() {
        return _size;
    }

    int length() {
        return _length;
    }
    
    Buffer& bind_base(GLuint index) {
        glBindBufferBase(target, index, id);
        return *this;
    }

    template <typename T>
    void set_data(std::vector<T> data, GLenum usage = GL_STATIC_DRAW) {
        glBindBuffer(target, id);
        glBufferData(target, sizeof(T) * data.size(), data.data(), usage);
        glBindBuffer(target, 0); // unbind
        _length = data.size();
        _size = data.size() * sizeof(T);
    } 
};

class VAO {
    const std::unordered_map<GLenum, GLuint> type_size{{
        {GL_BYTE, 1},
        {GL_UNSIGNED_BYTE, 1},
        {GL_SHORT, 2},
        {GL_UNSIGNED_SHORT, 2},
        {GL_INT, 4},
        {GL_UNSIGNED_INT, 4},
        {GL_HALF_FLOAT, 2},
        {GL_FLOAT, 4},
        {GL_DOUBLE, 8},
        {GL_FIXED, 4}
    }};
    int auto_attrib_counter = 0;

public:
    GLuint id;

    VAO() {
        // create VAO, which encapsulates the vertex buffer and its layout
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Array_Object
        glGenVertexArrays(1, &id);
    }

    ~VAO() {
        glDeleteVertexArrays(1, &id);
    }

    void bind() {
        glBindVertexArray(id);
    }

    void unbind() {
        glBindVertexArray(0);
    }

    VAO& attrib_index(GLuint index) {
        auto_attrib_counter = index;
        return *this;
    }

    VAO& bind_attrib(const Buffer& buffer, GLuint num_components, GLenum type, GLuint divisor = NOT_INSTANCED) {
        return bind_attrib(buffer, 0, type_size.at(type) * num_components, num_components, type, divisor);
    }

    VAO& bind_attrib(const Buffer& buffer, uint offset_bytes, GLuint stride, GLuint num_components, GLenum type, GLuint divisor) {
        bind();
        const GLuint i = auto_attrib_counter;
        if (i >= GL_MAX_VERTEX_ATTRIBS) {
            std::stringstream str;
            str << "Current attribute index " << i;
            str << " is >= max of " << GL_MAX_VERTEX_ATTRIBS;
            throw std::runtime_error(str.str());
        }

        glEnableVertexAttribArray(i);

        // binds the VBO to a VAO attribute
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Separate_attribute_format
        glBindVertexBuffer(
            i, // binding (attribute) index
            buffer.id, // buffer
            offset_bytes, // offset of first element
            stride // stride
        );
        
        glVertexAttribFormat(
            i, // attribute index
            num_components, // size (number of components)
            type, // type of components
            GL_FALSE, // normalize integer type?
            0 // "added to the buffer binding's offset to get the offset for this attribute"
        );

        glVertexBindingDivisor(i, divisor);
        ++auto_attrib_counter;
        unbind();
        return *this;
    }
};
}
