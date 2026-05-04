#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <vector>

#ifndef PI
#define PI 3.141592f
#endif

// MagneticField: Traces dipole field lines around a tilted magnetic axis, scaled by a B-field strength proxy derived from mass and rotation period.
struct MagneticField
{
    struct FieldLine
    {
        unsigned int VAO, VBO;
        int pointCount;
    };

    std::vector<FieldLine> lines;

    // Magnetic axis is intentionally tilted to mimic a real pulsar
    glm::vec3 axis = glm::normalize(glm::vec3(0.3f, 1.0f, 0.0f));

    void generate(float starRadius, float massSolar, float periodSeconds,
                  int numLines = 12, int pointsPerLine = 150)
    {
        cleanup(); // Free any previously generated GPU buffers

        // Build an orthonormal basis around the magnetic axis
        glm::vec3 up = axis;
        glm::vec3 perp = glm::normalize(glm::cross(up, glm::vec3(0.0f, 0.0f, 1.0f)));
        glm::vec3 perp2 = glm::normalize(glm::cross(up, perp));

        // B = sqrt(M / P), normalized to a 1.4 solar mass, 1-second reference pulsar.
        // Higher mass or faster spin -> stronger field -> field lines extend further.
        float massNorm = massSolar / 1.4f;
        float periodNorm = periodSeconds / 1.0f;
        float fieldStrength = sqrtf(massNorm / periodNorm);

        // R0: maximum radial reach of the field lines, clamped to a sane range
        float R0 = glm::clamp(
            starRadius * 3.5f * fieldStrength,
            starRadius * 1.5f,
            starRadius * 8.0f
        );

        for (int i = 0; i < numLines; i++)
        {
            float phi = (float)i / numLines * 2.0f * PI;
            std::vector<glm::vec3> pts;

            // Parametric dipole field line: r = R0 * sin2(theta)
            for (int j = 0; j <= pointsPerLine; j++)
            {
                float theta = glm::radians(5.0f) + (float)j / pointsPerLine * glm::radians(170.0f);
                float r = R0 * sinf(theta) * sinf(theta);

                float x_local = r * sinf(theta) * cosf(phi);
                float y_local = r * cosf(theta);
                float z_local = r * sinf(theta) * sinf(phi);

                // Rotate from magnetic-axis-aligned space into world space
                glm::vec3 worldPos = x_local * perp + y_local * up + z_local * perp2;
                pts.push_back(worldPos);
            }

            FieldLine fl;
            fl.pointCount = pts.size();
            glGenVertexArrays(1, &fl.VAO);
            glGenBuffers(1, &fl.VBO);
            glBindVertexArray(fl.VAO);
            glBindBuffer(GL_ARRAY_BUFFER, fl.VBO);
            glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3), pts.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
            glEnableVertexAttribArray(0);
            glBindVertexArray(0);
            lines.push_back(fl);
        }
    }

    void render()
    {
        for (auto& fl : lines)
        {
            glBindVertexArray(fl.VAO);
            glDrawArrays(GL_LINE_STRIP, 0, fl.pointCount);
            glBindVertexArray(0);
        }
    }

    void cleanup()
    {
        for (auto& fl : lines)
        {
            glDeleteVertexArrays(1, &fl.VAO);
            glDeleteBuffers(1, &fl.VBO);
        }
        lines.clear();
    }
};
