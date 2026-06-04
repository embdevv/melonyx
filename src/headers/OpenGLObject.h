// headers/OpenGLObject.h
#pragma once
#include <iostream>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include "tiny_obj_loader.h"

class OpenGLObject {
public:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color    = glm::vec3(1.0f);

protected:
    GLuint VAO = 0, VBO = 0, EBO = 0;
    std::vector<float>    vertices;
    std::vector<unsigned> indices;

public:
    virtual void upload() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
            vertices.size() * sizeof(float),
            vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned),
            indices.data(), GL_STATIC_DRAW);

        // position attribute (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // normal attribute (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    virtual void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(),
            GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    virtual void cleanup() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    virtual ~OpenGLObject() = default;
};


class ObjMesh : public OpenGLObject {
public:
    bool load(const std::string& path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
            std::cerr << "Failed to load OBJ: " << err << "\n";
            return false;
        }

        vertices.clear();
        indices.clear();

        // Flatten all faces into interleaved position + normal
        for (auto& shape : shapes) {
            for (auto& idx : shape.mesh.indices) {
                // position
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                // normal
                if (idx.normal_index >= 0) {
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 0]);
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 1]);
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 2]);
                }
                else {
                    vertices.push_back(0); vertices.push_back(1); vertices.push_back(0);
                }
                indices.push_back((unsigned)indices.size());
            }
        }
        return true;
    }
};