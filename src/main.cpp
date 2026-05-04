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

#define VERTEX_SHADER SHADER_DIR "vertex_shader.glsl"
#define FRAGMENT_SHADER SHADER_DIR "fragment_shader.glsl"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

glm::vec3 cameraPos = glm::vec3(2.5f, 2.5f, 10.0);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

const double solarMass = 1.980e30;
const float PI = 3.141592f;


struct NeutronStar{

    float radius;
    double mass;
    float period;
    float rotationSpeed;
    glm::vec3 rotationAxis;
    glm::vec3 position;

    //for future bloom stuff
    glm::vec3 color;
    float emissionStrength;
    float pulseFrequency;
    float pulsePhase;

    // Sphere gen storage
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<int> indices;
    std::vector<int> lineIndices;
    int sectors, stacks;
    
    unsigned int VBO, VAO, EBO, normalVBO, texVBO;

    NeutronStar(float r, double m, int sec, int stk, float per)
        : radius(r), mass(m * solarMass), sectors(sec), stacks(stk), period(per),
          rotationSpeed((2.0f * PI) / period), rotationAxis(-1.0f, 1.0f, 0.0f),
          position(0.0f, 0.0f, 0.0f),
          color(0.6f, 0.8f, 1.0f),
          emissionStrength(1.0f),
          pulseFrequency(1.0f),
          pulsePhase(0.0f) {}

    void generate(){
        // Calculating vertices, normals, and tex for sphere
        float x, y, z, xy;
        float nx, ny, nz, lengthInv = 1.0f / radius;
        float s, t;
        float sectorStep = 2 * PI / sectors;
        float stackStep = PI / stacks;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stacks; i++){
            stackAngle = PI / 2 - i * stackStep;
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);

            for (int j = 0; j <= sectors; j++){
                sectorAngle = j * sectorStep;

                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

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

        
        // Indices
        int k1, k2;
        for (int i = 0; i < stacks; ++i){
            k1 = i * (sectors + 1);
            k2 = k1 + sectors + 1;

            for (int j = 0; j < sectors; ++j, ++k1, ++k2){
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


    void setupBuffers(){

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glGenBuffers(1, &normalVBO);
        glGenBuffers(1, &texVBO); glBindVertexArray(VAO); 

        glBindBuffer(GL_ARRAY_BUFFER, VBO); 
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW); 
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);


        glBindBuffer(GL_ARRAY_BUFFER, texVBO);
        glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), texCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void render(){
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void update(float deltaTime){
        // later use for other stuff lol
        pulsePhase += pulseFrequency * deltaTime;
    }

    void cleanup(){
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &normalVBO);
        glDeleteBuffers(1, &texVBO);

    }
};

struct Cone
{
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, normalVBO, EBO;

    void generate(float radius, float height, int segments)
    {
        vertices.clear();
        normals.clear();
        indices.clear();

        // Apex at origin
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);

        normals.push_back(0.0f);
        normals.push_back(1.0f);
        normals.push_back(0.0f);

        // Base ring at -height
        for (int i = 0; i <= segments; i++)
        {
            float angle = (float)i / segments * 2.0f * PI;
            float x = cos(angle) * radius;
            float z = sin(angle) * radius;

            vertices.push_back(x);
            vertices.push_back(-height);
            vertices.push_back(z);

            glm::vec3 normal = glm::normalize(glm::vec3(x, radius / height, z));

            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);
        }

        // Cone side triangles
        for (int i = 1; i <= segments; i++)
        {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    }

    void setupBuffers(){

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &normalVBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO); 
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW); 
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void render(){
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup(){
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &normalVBO);
        glDeleteBuffers(1, &EBO);
    }
};


struct SpacetimeGrid {
    std::vector<float> vertices;   // pos + uv
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    void generate(int size, float spacing, float lineWidth, int subdivisions = 40) {
        float hw = lineWidth * 0.5f;
        float total = size * spacing;
        float start = -total * 0.5f;
        float end   =  total * 0.5f;

        auto addQuadStrip = [&](bool alongX, float fixedCoord) {
            int steps = size * subdivisions;
            float stepSize = total / steps;

            for (int s = 0; s < steps; s++) {
                float t0 = start + s * stepSize;
                float t1 = t0 + stepSize;

                unsigned int base = vertices.size() / 3;

                glm::vec3 a, b, c, d;
                if (alongX) {
                    // line runs along X, fixed Z = fixedCoord
                    a = {t0, 0, fixedCoord - hw};
                    b = {t1, 0, fixedCoord - hw};
                    c = {t1, 0, fixedCoord + hw};
                    d = {t0, 0, fixedCoord + hw};
                } else {
                    // line runs along Z, fixed X = fixedCoord
                    a = {fixedCoord - hw, 0, t0};
                    b = {fixedCoord - hw, 0, t1};
                    c = {fixedCoord + hw, 0, t1};
                    d = {fixedCoord + hw, 0, t0};
                }

                for (auto& v : {a, b, c, d}) {
                    vertices.push_back(v.x);
                    vertices.push_back(v.y);
                    vertices.push_back(v.z);
                }
                indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
                indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
            }
        };

        for (int i = 0; i <= size; i++) {
            float t = start + i * spacing;
            addQuadStrip(true,  t);  // lines along X
            addQuadStrip(false, t);  // lines along Z
        }
    }

    void setupBuffers() {
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

    void render() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
};

struct MagneticField {
    struct FieldLine {
        unsigned int VAO, VBO;
        int pointCount;
    };
    std::vector<FieldLine> lines;

    // magnetic axis — tilted like a real pulsar
    glm::vec3 axis = glm::normalize(glm::vec3(0.3f, 1.0f, 0.0f));

    void generate(float starRadius, float massSolar, float periodSeconds,
                  int numLines = 12, int pointsPerLine = 150)
    {
        // Clean up previous geometry if regenerating
        cleanup();

        glm::vec3 up    = axis;
        glm::vec3 perp  = glm::normalize(glm::cross(up, glm::vec3(0.0f, 0.0f, 1.0f)));
        glm::vec3 perp2 = glm::normalize(glm::cross(up, perp));

        // B field strength proxy: heavier + faster spinning = stronger field
        // Physically: B ∝ sqrt(M / P), normalized around a 1.4 solar mass, 0.0014s pulsar
        float massNorm = massSolar / 1.4f;
        float periodNorm = periodSeconds / 1.0f; // reference: fast millisecond pulsar
        float fieldStrength = sqrtf(massNorm / periodNorm); // stronger = further reach

        // R0: how far field lines extend. Clamp so it never looks crazy
        float R0 = glm::clamp(starRadius * 3.5f * fieldStrength, starRadius * 1.5f, starRadius * 8.0f);

        for (int i = 0; i < numLines; i++) {
            float phi = (float)i / numLines * 2.0f * PI;
            std::vector<glm::vec3> pts;

            for (int j = 0; j <= pointsPerLine; j++) {
                float theta = glm::radians(5.0f) + (float)j / pointsPerLine * glm::radians(170.0f);
                float r = R0 * sinf(theta) * sinf(theta);

                float x_local = r * sinf(theta) * cosf(phi);
                float y_local = r * cosf(theta);
                float z_local = r * sinf(theta) * sinf(phi);

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

    void render() {
        for (auto& fl : lines) {
            glBindVertexArray(fl.VAO);
            glDrawArrays(GL_LINE_STRIP, 0, fl.pointCount);
            glBindVertexArray(0);
        }
    }

    void cleanup() {
        for (auto& fl : lines) {
            glDeleteVertexArrays(1, &fl.VAO);
            glDeleteBuffers(1, &fl.VBO);
        }
        lines.clear();
    }
};

audio::AudioEngine gAudio;
SimulationUI ui;

int main()
{
    // CORE: very important in all projects
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
        std::cout << "Failed to initialize GLAD" << std::endl; return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    gAudio.start();
    // CORE

    Shader starShader(VERTEX_SHADER, FRAGMENT_SHADER);
    Shader skyboxShader(SHADER_DIR "skybox_vertex.glsl", SHADER_DIR "skybox_fragment.glsl");
    Shader beamShader(VERTEX_SHADER, SHADER_DIR "beam_fragment.glsl");
    
    NeutronStar star(1.0f, 1.4f, 128, 64, .0014f);
    star.generate();
    star.setupBuffers();

    Cone beamCone;
    beamCone.generate(0.2f, 5.0f, 64);
    beamCone.setupBuffers();

    Shader gridShader(SHADER_DIR "grid_vertex.glsl", SHADER_DIR "grid_fragment.glsl");
    SpacetimeGrid grid;
    grid.generate(50, 0.5f, 0.02f); // 50x50, 0.5 unit spacing
    grid.setupBuffers();

    Shader magShader(SHADER_DIR "magfield_vertex.glsl", SHADER_DIR "magfield_fragment.glsl");
    MagneticField magField;
    float lastMagMass   = ui.starMassSolar;
    float lastMagPeriod = ui.rotationPeriod;
    float lastMagRadius = ui.starRadius;
    magField.generate(star.radius, ui.starMassSolar, ui.rotationPeriod, 16, 150);

    // HDR Framebuffer Post Processing
    // Initialize our custom HDR Framebuffer
    Framebuffer hdrFBO(SCR_WIDTH, SCR_HEIGHT);
    BloomRenderer bloom(SCR_WIDTH, SCR_HEIGHT); // NEW: Handles all post-processing setup

    // PROCEDURAL SKYBOX SETUP

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    

    // Render Loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-Frame Time Logic & Input
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        process_input(window);

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
        ImGui::SliderFloat("Beam Radius", &ui.beamRadius, 0.05f, 2.0f);
        ImGui::SliderFloat("Beam Length", &ui.beamLength, 2.0f, 105.0f);
        ImGui::SliderFloat("Beam Speed", &ui.beamSpeed, 0.1f, 10.0f);
        ImGui::SliderFloat("Beam Intensity", &ui.beamIntensity, 1.0f, 40.0f);
        ImGui::SliderFloat("Beam Alpha", &ui.beamAlpha, 0.1f, 1.5f);
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

        // Show the current field reach for feedback
        float massNorm = ui.starMassSolar / 1.4f;
        float periodNorm = ui.rotationPeriod / 1.0f;
        float fieldStr = sqrtf(massNorm / periodNorm);
        float R0_preview = glm::clamp(ui.starRadius * 3.5f * fieldStr, ui.starRadius * 1.5f, ui.starRadius * 8.0f);
        ImGui::Text("Field Reach: %.2f units", R0_preview);

        ImGui::Text("Press ESC to toggle mouse capture.");

        ImGui::End();

        gAudio.syncFromUi(ui);

        // PASS 1: RENDER SCENE TO HDR FRAMEBUFFER
        hdrFBO.bind(); // Draw to our custom buffers, not the screen
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Render Neutron Star
        starShader.use();
        starShader.setVec3("viewPos", cameraPos);
        starShader.setFloat("time", currentFrame);
        starShader.setVec3("starColor", ui.starColor);
        starShader.setFloat("emissiveStrength", ui.emissionStrength);

        star.update(deltaTime);

        star.radius = ui.starRadius;
        star.mass = ui.starMassSolar * solarMass;
        star.period = ui.rotationPeriod;
        star.rotationSpeed = (2.0f * PI) / star.period;
        star.color = ui.starColor;
        star.emissionStrength = ui.emissionStrength;

        // Matrix Math for the Star
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, star.rotationSpeed * currentFrame, star.rotationAxis);
        model = glm::scale(model, glm::vec3(ui.starRadius));
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        starShader.setMat4("model", model);
        starShader.setMat4("view", view);
        starShader.setMat4("projection", projection);

        star.render();



        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Regenerate field lines if physical params changed
        auto floatChanged = [](float a, float b) { return fabsf(a - b) > 1e-5f; };

        if (floatChanged(ui.starMassSolar,  lastMagMass)   ||
            floatChanged(ui.rotationPeriod, lastMagPeriod) ||
            floatChanged(ui.starRadius,     lastMagRadius))
        {
            magField.generate(star.radius, ui.starMassSolar, ui.rotationPeriod, 16, 150);
            lastMagMass   = ui.starMassSolar;
            lastMagPeriod = ui.rotationPeriod;
            lastMagRadius = ui.starRadius;
        }

        if (ui.magFieldVisible)
        {
            magShader.use();
            glm::mat4 magModel = glm::mat4(1.0f);
            magModel = glm::rotate(magModel, star.rotationSpeed * currentFrame, star.rotationAxis);
            magShader.setMat4("model", magModel);
            magShader.setMat4("view", view);
            magShader.setMat4("projection", projection);
            magShader.setVec3("fieldColor", glm::vec3(0.2f, 0.6f, 1.0f));
            magShader.setFloat("fieldAlpha", 0.5f);
            magField.render();
        }

        
        /* Beam Rendering */
        beamShader.use();
        beamShader.setFloat("time", currentFrame);

        float pulsePhase = sin(currentFrame * ui.beamSpeed * 2.0f * PI) * 0.5f + 0.5f;

        beamShader.setFloat("pulsePhase", pulsePhase);
        beamShader.setVec3("beamColor", ui.beamColor);
        beamShader.setFloat("beamIntensity", ui.beamIntensity);
        beamShader.setFloat("beamAlpha", ui.beamAlpha);
        beamShader.setVec3("viewPos", cameraPos);

        auto renderBeam = [&](bool north, float radiusScale, float lengthScale, float layerAlpha)
            {
                beamShader.setFloat("layerAlpha", layerAlpha);

                glm::mat4 beamModel = glm::mat4(1.0f);

                beamModel = glm::rotate(
                    beamModel,
                    star.rotationSpeed * currentFrame,
                    star.rotationAxis
                );

                if (north)
                {
                    beamModel = glm::translate(
                        beamModel,
                        glm::vec3(0.0f, ui.starRadius, 0.0f)
                    );

                    beamModel = glm::rotate(
                        beamModel,
                        PI,
                        glm::vec3(1.0f, 0.0f, 0.0f)
                    );
                }
                else
                {
                    beamModel = glm::translate(
                        beamModel,
                        glm::vec3(0.0f, -ui.starRadius, 0.0f)
                    );
                }

                beamModel = glm::scale(
                    beamModel,
                    glm::vec3(
                        (ui.beamRadius / 0.2f) * radiusScale,
                        (ui.beamLength / 5.0f) * lengthScale,
                        (ui.beamRadius / 0.2f) * radiusScale
                    )
                );

                beamShader.setMat4("model", beamModel);
                beamShader.setMat4("view", view);
                beamShader.setMat4("projection", projection);

                beamCone.render();
            };

        glDepthMask(GL_FALSE);

        // Outer glow layer
        renderBeam(true, 2.4f, 1.25f, 0.45f);
        renderBeam(false, 2.4f, 1.25f, 0.45f);

        // Main beam body
        renderBeam(true, 1.35f, 1.10f, 0.75f);
        renderBeam(false, 1.35f, 1.10f, 0.75f);

        // Bright inner core
        renderBeam(true, 0.55f, 1.00f, 1.25f);
        renderBeam(false, 0.55f, 1.00f, 1.25f);

        glDepthMask(GL_TRUE);

        /* Beam Rendering */

        // Grid Rendering
        if (ui.gridVisible) 
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            gridShader.use();
            gridShader.setMat4("view", view);
            gridShader.setMat4("projection", projection);
            gridShader.setVec3("starPos", star.position);
            // Drive warp depth from mass — heavier star = deeper well
            float gravStrength = 5.0f * (ui.starMassSolar / 1.4f);
            gridShader.setFloat("gravStrength", gravStrength);
            gridShader.setFloat("gravSoftening", ui.starRadius * 0.5f);

            grid.render();
        }

        glDisable(GL_BLEND);


        // Render Procedural Skybox
        glDepthFunc(GL_LEQUAL);  // Draw skybox behind everything else
        skyboxShader.use();
        
        glm::mat4 skyView = glm::mat4(glm::mat3(view)); // Remove translation so stars don't move
        skyboxShader.setMat4("view", skyView);
        skyboxShader.setMat4("projection", projection);
        
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        
        glDepthFunc(GL_LESS); // Restore default depth testing

        // Done writing to custom FBO
        hdrFBO.unbind();

        // ==========================================
        // GRAVITATIONAL LENSING UNIFORMS
        // ==========================================
        // Project the star's world position into screen-space UV
        glm::vec4 starClip = projection * view * glm::vec4(star.position, 1.0f);
        glm::vec2 starScreenPos(0.5f);
        if (starClip.w > 0.0f) {
            glm::vec3 starNDC = glm::vec3(starClip) / starClip.w;
            starScreenPos = glm::vec2(starNDC.x * 0.5f + 0.5f, starNDC.y * 0.5f + 0.5f);
        }

        // Apparent radius in vertical UV units (small-angle approximation)
        float distToStar = glm::length(cameraPos - star.position);
        float apparentRadiusUV = 0.0f;
        if (distToStar > star.radius) {
            float halfFovY = glm::radians(fov) * 0.5f;
            apparentRadiusUV = (star.radius / distToStar) / tanf(halfFovY) * 0.5f;
        }

        bloom.finalShader->use();
        bloom.finalShader->setVec2("starScreenPos", starScreenPos);
        bloom.finalShader->setFloat("starApparentRadius", apparentRadiusUV);

        float compactness = (ui.starMassSolar / 1.4f) / ui.starRadius;

        bloom.finalShader->setFloat("lensStrength", ui.lensStrength);
        bloom.finalShader->setFloat("lensCompactness", compactness);
        bloom.finalShader->setFloat("aspectRatio", (float)SCR_WIDTH / (float)SCR_HEIGHT);

        // PASS 2 & 3: BLOOM BLUR & FINAL COMPOSITE
        bloom.renderBloom(hdrFBO.colorBuffers[0], hdrFBO.colorBuffers[1]);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
bool cursorCaptured = true;

void process_input(GLFWwindow* window)
{
    float cameraSpeed = 2.5f * deltaTime;
    static bool escPressed = false;
    static bool gPressed = false;
    static bool mPressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        if (!escPressed)
        {
            cursorCaptured = !cursorCaptured;
            if (cursorCaptured)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true;
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


    // Grid Toggle
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

    // MagField Toggle
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
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;

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

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

