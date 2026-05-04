#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <math.h>
#include <vector>

const double solarMass = 1.980e30;
const float PI = 3.141592f;


// NeutronStar: Procedurally generates and renders a UV sphere representing the neutron star. Also tracks physical simulation params.
struct NeutronStar
{
    float radius;
    double mass;
    float period;
    float rotationSpeed;
    glm::vec3 rotationAxis;
    glm::vec3 position;

    // Visual / bloom properties
    glm::vec3 color;
    float emissionStrength;
    float pulseFrequency;
    float pulsePhase;

    // Generated geometry
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<int> indices;
    std::vector<int> lineIndices;
    int sectors, stacks;

    unsigned int VBO, VAO, EBO, normalVBO, texVBO;

    NeutronStar(float r, double m, int sec, int stk, float per)
        : radius(r), mass(m * solarMass), sectors(sec), stacks(stk), period(per),
          rotationSpeed((2.0f * PI) / period),
          rotationAxis(-1.0f, 1.0f, 0.0f),
          position(0.0f, 0.0f, 0.0f),
          color(0.6f, 0.8f, 1.0f),
          emissionStrength(1.0f),
          pulseFrequency(1.0f),
          pulsePhase(0.0f)
    {}

    // Builds vertex positions, normals, tex-coords, and triangle/line indices
    // using the standard UV-sphere parametric equations.
    void generate()
    {
        float x, y, z, xy;
        float nx, ny, nz, lengthInv = 1.0f / radius;
        float s, t;
        float sectorStep = 2 * PI / sectors;
        float stackStep = PI / stacks;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stacks; i++)
        {
            stackAngle = PI / 2 - i * stackStep;
            xy = radius * cosf(stackAngle);
            z  = radius * sinf(stackAngle);

            for (int j = 0; j <= sectors; j++)
            {
                sectorAngle = j * sectorStep;

                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // Normals are just the normalized position on a unit sphere
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                normals.push_back(nx);
                normals.push_back(ny);
                normals.push_back(nz);

                s = (float)j / sectors;
                t = (float)i / stacks;
                texCoords.push_back(s);
                texCoords.push_back(t);
            }
        }

        // Build triangle indices. k1/k2 are the row offsets for adjacent stacks.
        // Skip degenerate triangles at the top (i==0) and bottom (i==stacks-1) poles.
        int k1, k2;
        for (int i = 0; i < stacks; ++i)
        {
            k1 = i * (sectors + 1);
            k2 = k1 + sectors + 1;

            for (int j = 0; j < sectors; ++j, ++k1, ++k2)
            {
                if (i != 0)
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                if (i != (stacks - 1))
                {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }

                lineIndices.push_back(k1);
                lineIndices.push_back(k2);
                if (i != 0)
                {
                    lineIndices.push_back(k1);
                    lineIndices.push_back(k1 + 1);
                }
            }
        }
    }

    void setupBuffers()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glGenBuffers(1, &normalVBO);
        glGenBuffers(1, &texVBO);

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

        // Attrib 2: tex coords
        glBindBuffer(GL_ARRAY_BUFFER, texVBO);
        glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), texCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void render()
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void update(float deltaTime)
    {
        pulsePhase += pulseFrequency * deltaTime;
    }

    void cleanup()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &normalVBO);
        glDeleteBuffers(1, &texVBO);
    }
};
