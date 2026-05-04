#pragma once

#include <glm/glm.hpp>

struct SimulationUI
{
    float lensStrength = 0.25f;

    float beamRadius = 0.25f;
    float beamLength = 8.0f;
    float beamSpeed = 1.5f;
    float beamIntensity = 12.0f;
    float beamAlpha = 0.75f;

    float starRadius = 1.0f;
    float starMassSolar = 1.4f;
    float rotationPeriod = 5.0f;
    float emissionStrength = 1.0f;

    glm::vec3 starColor = glm::vec3(0.6f, 0.8f, 1.0f);
    glm::vec3 beamColor = glm::vec3(0.6f, 0.8f, 1.0f);

    float masterVolume = 0.65f;
    bool audioEnabled = true;

    bool gridVisible = false;
    bool magFieldVisible = false;
};
