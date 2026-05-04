#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <math.h>
#include <vector>
#include "shaders/shader.h"
#include "Framebuffer.h"
#include "BloomRenderer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "SimulationUI.h"
#include "audio/AudioEngine.h"
#include "NeutronStar.h"
#include "Cone.h"
#include "SpacetimeGrid.h"
#include "MagneticField.h"

#define VERTEX_SHADER   SHADER_DIR "vertex_shader.glsl"
#define FRAGMENT_SHADER SHADER_DIR "fragment_shader.glsl"

// Callbacks & Input
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Camera State
glm::vec3 cameraPos = glm::vec3(2.5f, 2.5f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f,  0.0f);

int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Globals
audio::AudioEngine gAudio;
SimulationUI ui;

int main()
{
    // GLFW / OpenGL context setup
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "neutron-star", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    gAudio.start();

    // Shader loading
    Shader starShader(VERTEX_SHADER, FRAGMENT_SHADER);
    Shader skyboxShader(SHADER_DIR "skybox_vertex.glsl", SHADER_DIR "skybox_fragment.glsl");
    Shader beamShader(VERTEX_SHADER, SHADER_DIR "beam_fragment.glsl");
    Shader gridShader(SHADER_DIR "grid_vertex.glsl", SHADER_DIR "grid_fragment.glsl");
    Shader magShader(SHADER_DIR "magfield_vertex.glsl", SHADER_DIR "magfield_fragment.glsl");

    // Scene object setup
    NeutronStar star(1.0f, 1.4f, 128, 64, 0.0014f);
    star.generate();
    star.setupBuffers();

    Cone beamCone;
    beamCone.generate(ui.beamRadius, ui.beamLength, 32);
    beamCone.setupBuffers();

    SpacetimeGrid grid;
    grid.generate(50, 0.5f, 0.02f); // 50x50 cells, 0.5-unit spacing
    grid.setupBuffers();

    MagneticField magField;
    float lastMagMass = ui.starMassSolar;
    float lastMagPeriod = ui.rotationPeriod;
    float lastMagRadius = ui.starRadius;
    magField.generate(star.radius, ui.starMassSolar, ui.rotationPeriod, 16, 150);

    // HDR framebuffer & bloom post-processing
    Framebuffer hdrFBO(SCR_WIDTH, SCR_HEIGHT);
    BloomRenderer bloom(SCR_WIDTH, SCR_HEIGHT);

    // Procedural skybox geometry (unit cube, 36 verts)
    float skyboxVertices[] = {
        // Back face
        -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
        // Left face
        -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
        // Right face
         1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
        // Front face
        -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
        // Top face
        -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,
        // Bottom face
        -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Render Loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame timing
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(window);

        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Neutron Star Controls");

        if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Master", &ui.masterVolume, 0.0f, 1.0f, "%.2f");
            ImGui::Checkbox("Enable generative audio", &ui.audioEnabled);
            if (!gAudio.deviceReady())
                ImGui::TextDisabled("OpenAL not ready. Try: brew install openal-soft");
        }

        ImGui::Separator();
        ImGui::Text("Gravitational Lensing");
        ImGui::SliderFloat("Lensing Strength", &ui.lensStrength, 0.0f, 1.0f);

        ImGui::Separator();
        ImGui::Text("Pulsar Beam");
        ImGui::SliderFloat("Beam Radius", &ui.beamRadius, 0.05f, 1.5f);
        ImGui::SliderFloat("Beam Length", &ui.beamLength, 1.0f, 105.0f);
        ImGui::SliderFloat("Beam Speed", &ui.beamSpeed, 0.1f, 10.0f);
        ImGui::SliderFloat("Beam Intensity", &ui.beamIntensity, 0.0f, 20.0f);
        ImGui::SliderFloat("Beam Alpha", &ui.beamAlpha, 0.0f,  1.0f);
        ImGui::ColorEdit3("Beam Color", glm::value_ptr(ui.beamColor));

        ImGui::Separator();
        ImGui::Text("Star Properties");
        ImGui::SliderFloat("Star Radius", &ui.starRadius, 0.3f, 3.0f);
        ImGui::SliderFloat("Star Mass", &ui.starMassSolar, 0.5f, 3.0f);

        float compactnessPreview = (ui.starMassSolar / 1.4f) / ui.starRadius;
        ImGui::Text("Compactness: %.2f", compactnessPreview);
        ImGui::Text("Mass affects lensing through mass / radius.");
        ImGui::Text("Effective Lensing: %.2f", ui.lensStrength * (ui.starMassSolar / 1.4f));
        ImGui::SliderFloat("Rotation Period", &ui.rotationPeriod, 0.5f, 20.0f);
        ImGui::SliderFloat("Emission Strength", &ui.emissionStrength, 0.0f, 5.0f);
        ImGui::ColorEdit3("Star Color", glm::value_ptr(ui.starColor));

        ImGui::Separator();
        ImGui::Text("Spacetime Grid");
        ImGui::Checkbox("Show Grid (G)", &ui.gridVisible);

        ImGui::Text("Magnetic Field");
        ImGui::Checkbox("Show Field (M)", &ui.magFieldVisible);
        ImGui::Text("Field reacts to Star Mass and Rotation Period.");

        // Live preview of computed field reach
        {
            float massNorm = ui.starMassSolar / 1.4f;
            float periodNorm = ui.rotationPeriod / 1.0f;
            float fieldStr = sqrtf(massNorm / periodNorm);
            float R0_preview = glm::clamp(
                ui.starRadius * 3.5f * fieldStr,
                ui.starRadius * 1.5f,
                ui.starRadius * 8.0f
            );
            ImGui::Text("Field Reach: %.2f units", R0_preview);
        }

        ImGui::Text("Press ESC to toggle mouse capture.");
        ImGui::End();

        gAudio.syncFromUi(ui);

        // Pass 1: Render scene to HDR framebuffer
        hdrFBO.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Shared MVP matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(
            glm::radians(fov),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f
        );

        // Sync star properties from UI
        star.update(deltaTime);
        star.radius = ui.starRadius;
        star.mass = ui.starMassSolar * solarMass;
        star.period = ui.rotationPeriod;
        star.rotationSpeed = (2.0f * PI) / star.period;
        star.color  = ui.starColor;
        star.emissionStrength = ui.emissionStrength;

        // Neutron star rendering
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, star.rotationSpeed * currentFrame, star.rotationAxis);
        model = glm::scale(model, glm::vec3(ui.starRadius));

        starShader.use();
        starShader.setVec3 ("viewPos", cameraPos);
        starShader.setFloat("time", currentFrame);
        starShader.setVec3 ("starColor", ui.starColor);
        starShader.setFloat("emissiveStrength", ui.emissionStrength);
        starShader.setMat4 ("model", model);
        starShader.setMat4 ("view", view);
        starShader.setMat4 ("projection", projection);
        star.render();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Regenerate magnetic field lines when physical params change
        auto floatChanged = [](float a, float b) { return fabsf(a - b) > 1e-5f; };

        if (floatChanged(ui.starMassSolar, lastMagMass) ||
            floatChanged(ui.rotationPeriod, lastMagPeriod) ||
            floatChanged(ui.starRadius, lastMagRadius))
        {
            magField.generate(star.radius, ui.starMassSolar, ui.rotationPeriod, 16, 150);
            lastMagMass = ui.starMassSolar;
            lastMagPeriod = ui.rotationPeriod;
            lastMagRadius = ui.starRadius;
        }

        // Magnetic field rendering
        if (ui.magFieldVisible)
        {
            glm::mat4 magModel = glm::mat4(1.0f);
            magModel = glm::rotate(magModel, star.rotationSpeed * currentFrame, star.rotationAxis);

            magShader.use();
            magShader.setMat4("model", magModel);
            magShader.setMat4("view", view);
            magShader.setMat4("projection", projection);
            magShader.setVec3("fieldColor", glm::vec3(0.2f, 0.6f, 1.0f));
            magShader.setFloat("fieldAlpha", 0.5f);
            magField.render();
        }

        // Pulsar beam rendering (north + south cones)
        {
            float pulsePhase = sin(currentFrame * ui.beamSpeed * 2.0f * PI) * 0.5f + 0.5f;

            beamShader.use();
            beamShader.setFloat("time", currentFrame);
            beamShader.setFloat("pulsePhase", pulsePhase);
            beamShader.setVec3("beamColor", ui.beamColor);
            beamShader.setFloat("beamIntensity", ui.beamIntensity);
            beamShader.setFloat("beamAlpha", ui.beamAlpha);
            beamShader.setMat4("view", view);
            beamShader.setMat4("projection", projection);

            // North beam
            glm::mat4 beamModel = glm::mat4(1.0f);
            beamModel = glm::rotate(beamModel, star.rotationSpeed * currentFrame, star.rotationAxis);
            beamModel = glm::translate(beamModel, glm::vec3(0.0f, 1.0f, 0.0f));
            beamModel = glm::scale(beamModel, glm::vec3(ui.beamRadius / 0.2f, ui.beamLength / 5.0f, ui.beamRadius / 0.2f));
            beamShader.setMat4("model", beamModel);
            beamCone.render();

            // South beam: same transform but flipped 180 around X
            beamModel = glm::mat4(1.0f);
            beamModel = glm::rotate(beamModel, star.rotationSpeed * currentFrame, star.rotationAxis);
            beamModel = glm::translate(beamModel, glm::vec3(0.0f, -1.0f, 0.0f));
            beamModel = glm::rotate(beamModel, PI, glm::vec3(1.0f, 0.0f, 0.0f));
            beamModel = glm::scale(beamModel, glm::vec3(ui.beamRadius / 0.2f, ui.beamLength / 5.0f, ui.beamRadius / 0.2f));
            beamShader.setMat4("model", beamModel);
            beamCone.render();
        }

        // Spacetime grid rendering
        if (ui.gridVisible)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Warp depth scales with mass: heavier star = deeper gravity well
            float gravStrength = 5.0f * (ui.starMassSolar / 1.4f);

            gridShader.use();
            gridShader.setMat4 ("view", view);
            gridShader.setMat4 ("projection", projection);
            gridShader.setVec3 ("starPos", star.position);
            gridShader.setFloat("gravStrength", gravStrength);
            gridShader.setFloat("gravSoftening", ui.starRadius * 0.5f);
            grid.render();
        }

        glDisable(GL_BLEND);

        // Procedural skybox
        glDepthFunc(GL_LEQUAL); // Pass depth test at far plane so skybox sits behind everything
        {
            glm::mat4 skyView = glm::mat4(glm::mat3(view)); // Strip translation so the skybox never moves

            skyboxShader.use();
            skyboxShader.setMat4("view", skyView);
            skyboxShader.setMat4("projection", projection);

            glBindVertexArray(skyboxVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }
        glDepthFunc(GL_LESS); // Restore default depth test

        hdrFBO.unbind();


        // Pass 2 & 3: Bloom blur + final composite with gravitational lensing applied in screen space

        // Project the star's world position to screen-space UV for the lensing shader
        glm::vec4 starClip = projection * view * glm::vec4(star.position, 1.0f);
        glm::vec2 starScreenPos(0.5f);
        if (starClip.w > 0.0f)
        {
            glm::vec3 starNDC = glm::vec3(starClip) / starClip.w;
            starScreenPos = glm::vec2(starNDC.x * 0.5f + 0.5f, starNDC.y * 0.5f + 0.5f);
        }

        // Apparent angular radius in vertical UV units (small-angle approximation)
        float distToStar = glm::length(cameraPos - star.position);
        float apparentRadiusUV = 0.0f;
        if (distToStar > star.radius)
        {
            float halfFovY = glm::radians(fov) * 0.5f;
            apparentRadiusUV = (star.radius / distToStar) / tanf(halfFovY) * 0.5f;
        }

        float compactness = (ui.starMassSolar / 1.4f) / ui.starRadius;

        bloom.finalShader->use();
        bloom.finalShader->setVec2("starScreenPos", starScreenPos);
        bloom.finalShader->setFloat("starApparentRadius", apparentRadiusUV);
        bloom.finalShader->setFloat("lensStrength", ui.lensStrength);
        bloom.finalShader->setFloat("lensCompactness", compactness);
        bloom.finalShader->setFloat("aspectRatio", (float)SCR_WIDTH / (float)SCR_HEIGHT);

        bloom.renderBloom(hdrFBO.colorBuffers[0], hdrFBO.colorBuffers[1]);

        // ImGui draw
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // Cleanup
    gAudio.stop();

    star.cleanup();
    hdrFBO.cleanup();
    bloom.cleanup();
    grid.cleanup();
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}


// Input & Callbacks
bool cursorCaptured = true;

void process_input(GLFWwindow* window)
{
    float cameraSpeed = 2.5f * deltaTime;

    // Use static booleans to detect key-press edges (fire once per press)
    static bool escPressed = false;
    static bool gPressed   = false;
    static bool mPressed   = false;

    // ESC toggles mouse capture / ImGui interaction mode
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        if (!escPressed)
        {
            cursorCaptured = !cursorCaptured;
            if (cursorCaptured)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true; // Prevent camera jump on re-capture
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            escPressed = true;
        }
    }
    else
    {
        escPressed = false;
    }

    // G: toggle spacetime grid
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        if (!gPressed)
        {
            ui.gridVisible = !ui.gridVisible;
            gPressed = true;
        }
    }
    else
    {
        gPressed = false;
    }

    // M: toggle magnetic field
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        if (!mPressed)
        {
            ui.magFieldVisible = !ui.magFieldVisible;
            mPressed = true;
        }
    }
    else
    {
        mPressed = false;
    }

    // WASD camera movement (only when cursor is captured)
    if (cursorCaptured)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    fov = glm::clamp(fov, 1.0f, 45.0f);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!cursorCaptured) return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = (xpos - lastX);
    float yoffset = (lastY - ypos); 
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;
    pitch = glm::clamp(pitch, -89.0f, 89.0f); // Prevent gimbal flip at poles

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}
