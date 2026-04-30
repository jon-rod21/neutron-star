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
#include "BloomRenderer.h" // NEW

#define VERTEX_SHADER SHADER_DIR "vertex_shader.glsl"
#define FRAGMENT_SHADER SHADER_DIR "fragment_shader.glsl"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

glm::vec3 cameraPos = glm::vec3(2.5f, 2.5f, 10.0);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
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
        glGenBuffers(1, &texVBO); glBindVertexArray(VAO); // VERY IMPORTANT, this activates the vao, ready for vbo's

        glBindBuffer(GL_ARRAY_BUFFER, VBO); // sets VBO as the current active buffer
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

int main()
{
    // CORE: very important in all projects
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(800, 600, "bruh", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);


    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl; return -1;
    }
    // CORE

    Shader starShader(VERTEX_SHADER, FRAGMENT_SHADER);
    Shader skyboxShader(SHADER_DIR "skybox_vertex.glsl", SHADER_DIR "skybox_fragment.glsl");
    
    NeutronStar star(1.0f, 1.4f, 128, 64, .0014f);
    star.generate();
    star.setupBuffers();

    // HDR Framebuffer Post Processing
    // Initialize our custom HDR Framebuffer
    Framebuffer hdrFBO(800, 600);
    BloomRenderer bloom(800, 600); // NEW: Handles all post-processing setup

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

        // ==========================================
        // PASS 1: RENDER SCENE TO HDR FRAMEBUFFER
        // ==========================================
        hdrFBO.bind(); // Draw to our custom buffers, not the screen
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // --- Render Neutron Star ---
        starShader.use();
        starShader.setVec3("viewPos", cameraPos);
        starShader.setFloat("time", currentFrame);

        star.update(deltaTime);

        // Matrix Math for the Star
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, star.rotationSpeed * currentFrame, star.rotationAxis);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);

        starShader.setMat4("model", model);
        starShader.setMat4("view", view);
        starShader.setMat4("projection", projection);

        star.render();

        // --- Render Procedural Skybox ---
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
        bloom.finalShader->setFloat("lensStrength", 0.5f);
        bloom.finalShader->setFloat("aspectRatio", 800.0f / 600.0f);

        // ==========================================
        // PASS 2 & 3: BLOOM BLUR & FINAL COMPOSITE
        // ==========================================
        bloom.renderBloom(hdrFBO.colorBuffers[0], hdrFBO.colorBuffers[1]);

        // ==========================================
        // SWAP BUFFERS AND POLL EVENTS
        // ==========================================
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ==========================================
    // RESOURCE CLEANUP
    // ==========================================
    star.cleanup();
    hdrFBO.cleanup();
    bloom.cleanup();
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}

void process_input(GLFWwindow* window)
{
    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
       cameraPos += cameraSpeed * cameraFront; 
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
       cameraPos -= cameraSpeed * cameraFront; 
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
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

