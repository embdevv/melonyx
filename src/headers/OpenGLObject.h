#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tiny_obj_loader.h"

class OpenGLObject {
public:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f);

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

        // Position attribute (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normal attribute (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    // No-argument draw for legacy/utility calls (e.g., render_particle.cpp)
    virtual void draw() {
        if (VAO == 0 || indices.empty()) return;

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // 3D Shader draw with uniforms
    virtual void draw(GLuint shaderProgram, const glm::mat4& model, const glm::mat4& proj, const glm::mat4& view, const glm::vec3& drawColor) {
        if (VAO == 0 || indices.empty()) return;

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), drawColor.r, drawColor.g, drawColor.b);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    virtual void cleanup() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
        VAO = 0; VBO = 0; EBO = 0;
    }

    virtual ~OpenGLObject() = default;
};

class ObjMesh : public OpenGLObject {
public:
    bool load(const std::string& path, const std::string& materialDir = "3D/") {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), materialDir.c_str());

        if (!warn.empty()) {
            std::cout << "[OBJ WARN] " << warn << "\n";
        }
        if (!err.empty()) {
            std::cerr << "[OBJ ERROR] " << err << "\n";
        }
        if (!ret) {
            std::cerr << "Failed to load OBJ: " << path << "\n";
            return false;
        }

        vertices.clear();
        indices.clear();

        for (auto& shape : shapes) {
            for (auto& idx : shape.mesh.indices) {
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

                if (idx.normal_index >= 0) {
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 0]);
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 1]);
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 2]);
                }
                else {
                    vertices.push_back(0.0f);
                    vertices.push_back(1.0f);
                    vertices.push_back(0.0f);
                }
                indices.push_back(static_cast<unsigned>(indices.size()));
            }
        }

        upload();
        std::cout << "[OBJ LOADED] " << path << " (" << indices.size() << " indices)\n";
        return true;
    }
};