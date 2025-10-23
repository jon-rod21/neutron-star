#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <sstream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
std::string readShaderFile(const char* filePath);

// VBO info 
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(1.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct NeutronStar{


    float radius;
    float mass;
    float rotationSpeed;
    glm::vec3 rotationAxis;
    glm::vec3 position;

    //for future bloom stuff
    glm::vec3 color;
    float emissionStrength;
    float pulseFrequency;
    float pulsePhase;

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<int> indices;
    std::vector<int> lineIndices;
    int sectors, stacks;
    
    unsigned int VBO, VAO, EBO, normalVBO, texVBO;

    NeutronStar(float r, float m, int sec, int stk)
        : radius(r), mass(m), sectors(sec), stacks(stk),
          rotationSpeed(1.0f), rotationAxis(0.0f, 1.0f, 0.0f),
          position(0.0f, 0.0f, 0.0f),
          color(0.6f, 0.8f, 1.0f),
          emissionStrength(1.0f),
          pulseFrequency(1.0f),
          pulsePhase(0.0f) {}

    void generate(){
        float PI = 3.141592f;
        
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
        glGenBuffers(1, &texVBO);

        glBindVertexArray(VAO); // VERY IMPORTANT, this activates the vao, ready for vbo's

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

    

    std::string vertexCode = readShaderFile("/home/jon/Desktop/cpp/opengl/neutron-star/src/shaders/vertex_shader.glsl");
    std::string fragmentCode = readShaderFile("/home/jon/Desktop/cpp/opengl/neutron-star/src/shaders/fragment_shader.glsl");


    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // used by EBO to avoid processing overlapping vertices for larger drawings

    // heres the cool part: your vao is like a book shelf, holding your vbos, actual data, in memory

    NeutronStar star(1.0f, 1.4f, 128, 64);
    star.generate();
    star.setupBuffers();


    while (!glfwWindowShouldClose(window))
    {
        process_input(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
//        glDisable(GL_CULL_FACE);

        glUseProgram(shaderProgram);


        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        star.update(deltaTime);

        glm::mat4 model = glm::mat4(1.0f);

//        model = glm::rotate(model, glm::radians(60.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        
        model = glm::rotate(model, currentFrame, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glm::mat4 projection = glm::perspective(glm::radians(fov),
                                           800.0f / 600.0f,
                                           0.1f, 100.0f);

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        star.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // optional but good practice, deallocate unused resources
    star.cleanup();
    glDeleteProgram(shaderProgram);


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

std::string readShaderFile(const char* filePath)
{
    std::ifstream shaderFile;
    std::stringstream shaderStream;

    shaderFile.open(filePath);
    if (!shaderFile.is_open()){
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    return shaderStream.str();
}

