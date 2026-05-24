// headers/main.h  (or sphere.h)
#pragma once
#include "OpenGLObject.h"
#include <cmath>

class Sphere : public OpenGLObject {
public:
    void build(float radius, int sectors, int stacks) {
        vertices.clear();
        indices.clear();

        for (int i = 0; i <= stacks; i++) {
            float phi = M_PI / 2 - i * M_PI / stacks;
            for (int j = 0; j <= sectors; j++) {
                float theta = j * 2 * M_PI / sectors;
                float x = radius * cos(phi) * cos(theta);
                float y = radius * cos(phi) * sin(theta);
                float z = radius * sin(phi);
                // position
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                // normal (unit sphere, normal == position normalized)
                vertices.push_back(x / radius);
                vertices.push_back(y / radius);
                vertices.push_back(z / radius);
            }
        }

        for (int i = 0; i < stacks; i++) {
            for (int j = 0; j < sectors; j++) {
                int a = i * (sectors + 1) + j;
                int b = a + sectors + 1;
                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(a + 1);
                indices.push_back(b);
                indices.push_back(b + 1);
                indices.push_back(a + 1);
            }
        }
    }
};