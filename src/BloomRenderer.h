#ifndef BLOOM_RENDERER_H
#define BLOOM_RENDERER_H

#include <glad/glad.h>
#include "shaders/shader.h"
#include <iostream>

class BloomRenderer {
public:
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    unsigned int quadVAO, quadVBO;
    
    Shader* blurShader;
    Shader* finalShader;
    
    unsigned int blurAmount = 10;
    float exposure = 1.0f;

    BloomRenderer(int width, int height) {
        // 1. Load Shaders (Uses your CMake SHADER_DIR macro)
        blurShader = new Shader(SHADER_DIR "blur_vertex.glsl", SHADER_DIR "blur_fragment.glsl");
        finalShader = new Shader(SHADER_DIR "final_vertex.glsl", SHADER_DIR "final_fragment.glsl");

        // 2. Setup Ping-Pong Framebuffers
        glGenFramebuffers(2, pingpongFBO);
        glGenTextures(2, pingpongColorbuffers);
        for (unsigned int i = 0; i < 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
            
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "BloomRenderer:: PingPong Framebuffer not complete!" << std::endl;
        }

        // 3. Setup Screen Quad
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        // 4. Configure uniform samplers
        finalShader->use();
        finalShader->setInt("scene", 0);
        finalShader->setInt("bloomBlur", 1);
        blurShader->use();
        blurShader->setInt("image", 0);
    }

    // Call this after drawing your scene to the custom HDR FBO
    void renderBloom(unsigned int hdrSceneTexture, unsigned int hdrBrightTexture) {
        // PASS 2: PING-PONG GAUSSIAN BLUR
        bool horizontal = true, first_iteration = true;
        blurShader->use();
        glDisable(GL_DEPTH_TEST);
        
        for (unsigned int i = 0; i < blurAmount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]); 
            blurShader->setInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            // Read from extracted bright spots first, then from the previous blur pass
            glBindTexture(GL_TEXTURE_2D, first_iteration ? hdrBrightTexture : pingpongColorbuffers[!horizontal]); 
            
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            horizontal = !horizontal;
            if (first_iteration) first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 

        // PASS 3: FINAL COMPOSITE & TONE MAPPING
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        finalShader->use();
        finalShader->setFloat("exposure", exposure); 
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrSceneTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void cleanup() {
        glDeleteFramebuffers(2, pingpongFBO);
        glDeleteTextures(2, pingpongColorbuffers);
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
        delete blurShader;
        delete finalShader;
    }
};

#endif