#pragma once

#include <GL/glew.h> 
// ---------------

class Renderer {
public:
    Renderer(int screenWidth, int screenHeight, int gridSize);
    ~Renderer(); 

    void draw(float* densityData);

private:
    unsigned int createShader(const char* vertexSource, const char* fragmentSource);

    unsigned int shaderProgram;
    unsigned int textureID;
    unsigned int vaoID;

    int gridSize;
    unsigned char* textureData;
};
