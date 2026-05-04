#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>


// SpacetimeGrid: Renders a flat gravitational-warp grid on the XZ plane. Each grid line is a subdivided quad strip so the vertex
// shader can smoothly displace it into a gravity well.
struct SpacetimeGrid
{
    std::vector<float> vertices;  // positions (x, y, z)
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    void generate(int size, float spacing, float lineWidth, int subdivisions = 40)
    {
        float hw    = lineWidth * 0.5f;
        float total = size * spacing;
        float start = -total * 0.5f;

        // Adds a subdivided quad strip for one grid line.
        // alongX=true, line runs along X at a fixed Z coordinate.
        // alongX=false, line runs along Z at a fixed X coordinate.
        auto addQuadStrip = [&](bool alongX, float fixedCoord)
        {
            int   steps    = size * subdivisions;
            float stepSize = total / steps;

            for (int s = 0; s < steps; s++)
            {
                float t0 = start + s * stepSize;
                float t1 = t0 + stepSize;

                unsigned int base = vertices.size() / 3;

                glm::vec3 a, b, c, d;
                if (alongX)
                {
                    a = {t0, 0, fixedCoord - hw};
                    b = {t1, 0, fixedCoord - hw};
                    c = {t1, 0, fixedCoord + hw};
                    d = {t0, 0, fixedCoord + hw};
                }
                else
                {
                    a = {fixedCoord - hw, 0, t0};
                    b = {fixedCoord - hw, 0, t1};
                    c = {fixedCoord + hw, 0, t1};
                    d = {fixedCoord + hw, 0, t0};
                }

                for (auto& v : {a, b, c, d})
                {
                    vertices.push_back(v.x);
                    vertices.push_back(v.y);
                    vertices.push_back(v.z);
                }

                indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
                indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
            }
        };

        for (int i = 0; i <= size; i++)
        {
            float t = start + i * spacing;
            addQuadStrip(true, t);  // horizontal lines (along X)
            addQuadStrip(false, t);  // vertical lines (along Z)
        }
    }

    void setupBuffers()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

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
        glDeleteBuffers(1, &EBO);
    }
};
