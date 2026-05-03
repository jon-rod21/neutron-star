#pragma once

#include <glm/glm.hpp>

struct SimulationUI
{
    float lensStrength = 0.25f;

    float beamRadius = 0.2f;
    float beamLength = 5.0f;
    float beamSpeed = 2.0f;
    float beamIntensity = 5.0f;
    float beamAlpha = 0.3f;

    float starRadius = 1.0f;
    float starMassSolar = 1.4f;
    float rotationPeriod = 5.0f;
    float emissionStrength = 1.0f;

    glm::vec3 starColor = glm::vec3(0.6f, 0.8f, 1.0f);
    glm::vec3 beamColor = glm::vec3(0.6f, 0.8f, 1.0f);

    float masterVolume = 0.65f;
    bool audioEnabled = true;
};
