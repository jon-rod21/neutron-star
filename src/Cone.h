#pragma once

#include <glad/glad.h>
#include <math.h>
#include <vector>

#ifndef PI
#define PI 3.141592f
#endif

// Cone: Used for the north and south pulsar emission beams.
struct Cone
{
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, normalVBO, EBO;

    void generate(float radius, float height, int segments)
    {
        // Apex at origin
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        normals.push_back(0.0f);
        normals.push_back(1.0f);
        normals.push_back(0.0f);

        // Base ring vertices
        for (int i = 0; i <= segments; i++)
        {
            float angle = (float)i / segments * 2.0f * PI;
            float x = cos(angle) * radius;
            float z = sin(angle) * radius;

            vertices.push_back(x);
            vertices.push_back(-height);
            vertices.push_back(z);

            normals.push_back(0.0f);
            normals.push_back(-1.0f);
            normals.push_back(0.0f);
        }

        // Fan triangles from apex (index 0) to base ring
        for (int i = 1; i <= segments; i++)
        {
            indices.push_back(0);
            indices.push_back(1);
            indices.push_back(i + 1);
        }
    }

    void setupBuffers()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &normalVBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // Attrib 0: positions
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Attrib 1: normals
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void render()
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &normalVBO);
        glDeleteBuffers(1, &EBO);
    }
};
