#include "Renderer.h"
#include <iostream>
#include <vector>

//vertex shader
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoords;
    out vec2 TexCoords;
    void main() {
        TexCoords = aTexCoords;
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)";

//fragment

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec2 TexCoords;
    uniform sampler2D fluidTexture;

    // In Renderer.cpp, replace the fragment shader's main function:
void main() {
    float density = texture(fluidTexture, TexCoords).r;
    
    vec3 color = vec3(0.0, 0.0, 0.0); // Black background
    if (density > 0.0) {
        color.r = density * 0.2;        // Little bit of red
        color.g = density * 0.6;        // Cyan/electric blue
        color.b = density * 1.0;        // Full blue
    }
    
    FragColor = vec4(color, 1.0);
}
)";

Renderer::Renderer(int screenWidth, int screenHeight, int gridSize) {
    this->gridSize = gridSize;

    this->shaderProgram = createShader(vertexShaderSource, fragmentShaderSource);
    this->textureData = new unsigned char[gridSize * gridSize * 4];

    glGenTextures(1, &this->textureID);
    glBindTexture(GL_TEXTURE_2D, this->textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gridSize, gridSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int vbo, ebo;
    glGenVertexArrays(1, &this->vaoID);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(this->vaoID);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices , GL_STATIC_DRAW);


    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

Renderer::~Renderer() {
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &textureID);
    glDeleteVertexArrays(1, &vaoID);
    delete[] textureData;
}
void Renderer::draw(float* densityData) {
    int N = this->gridSize;

    float maxDensity = 0.0f;
    for (int i = 0; i < (N + 2) * (N + 2); i++) {
        if (densityData[i] > maxDensity) {
            maxDensity = densityData[i];
        }
    }
    std::cout << "Max density: " << maxDensity << std::endl;

    for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) {
            int idx = (x + y * N);
            int fluidIdx = (x + 1) + (y + 1) * (N + 2);

            float d = densityData[fluidIdx];
            if (d > 1.0f) d = 1.0f;
            if (d < 0.0f) d = 0.0f;
            unsigned char densityByte = (unsigned char)(d * 255.0f);

            textureData[idx * 4 + 0] = densityByte;
            textureData[idx * 4 + 1] = 0;
            textureData[idx * 4 + 2] = 0;
            textureData[idx * 4 + 3] = 255;

        }
    }

    glBindTexture(GL_TEXTURE_2D, this->textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, N, N, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

    glUseProgram(this->shaderProgram);
    glBindVertexArray(this->vaoID);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

unsigned int Renderer::createShader(const char* vertexSource, const char* fragmentSource) {
    int success;
    char infoLog[512];

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX" << infoLog << std::endl;
    }
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::FRAG SHADER" << std::endl;
    }
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::LINK" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;

}