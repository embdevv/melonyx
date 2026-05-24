#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class Sphere {
public:
    GLuint VAO, VBO, EBO;
    std::vector<float>        vertices;
    std::vector<unsigned int> indices;
    int indexCount;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(0.2f, 0.6f, 1.0f);

    void build(float radius = 1.0f, int sectors = 36, int stacks = 18)
    {
        vertices.clear();
        indices.clear();

        for (int i = 0; i <= stacks; i++) {
            float phi = glm::pi<float>() / 2.0f - i * (glm::pi<float>() / stacks);
            float y   = radius * sinf(phi);
            float xz  = radius * cosf(phi);

            for (int j = 0; j <= sectors; j++) {
                float theta = j * (2.0f * glm::pi<float>() / sectors);
                float x = xz * cosf(theta);
                float z = xz * sinf(theta);

                // Position
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                // Normal (same as position normalized for unit sphere)
                vertices.push_back(x / radius);
                vertices.push_back(y / radius);
                vertices.push_back(z / radius);
            }
        }

        for (int i = 0; i < stacks; i++) {
            int k1 = i * (sectors + 1);
            int k2 = k1 + sectors + 1;
            for (int j = 0; j < sectors; j++, k1++, k2++) {
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                if (i != stacks - 1) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }
        indexCount = indices.size();
    }

    void upload()
    {
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
            indices.size() * sizeof(unsigned int),
            indices.data(), GL_STATIC_DRAW);

        // Position (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normal (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
};