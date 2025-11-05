#include <iostream>

// --- CHANGED ---
#define GLEW_STATIC 
#include <GL/glew.h>  
#include <GLFW/glfw3.h> 

#include "FluidSim.h"
#include "Renderer.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int GRID_SIZE = 128;

bool mouseIsDown = false;
double lastMouseX = 0;
double lastMouseY = 0;
Fluidsim* g_fluid_Sim = nullptr; 


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouseIsDown = true;
        }
        else if (action == GLFW_RELEASE) {
            mouseIsDown = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (mouseIsDown && g_fluid_Sim != nullptr) {
        float velX = (float)(xpos - lastMouseX);
        float velY = (float)(ypos - lastMouseY);

        int gridX = (int)((xpos / SCREEN_WIDTH) * GRID_SIZE);
        int gridY = (int)((ypos / SCREEN_HEIGHT) * GRID_SIZE);

        g_fluid_Sim->addDensity(gridX, gridY, 500.0f);
        g_fluid_Sim->addVelocity(gridX, gridY, velX, velY);
    }
    lastMouseX = xpos;
    lastMouseY = ypos;
}

int main() {    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

  
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple CPU Fluid Sim (GLEW)", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    int value;
    glGetIntegerv(GL_DOUBLEBUFFER, &value);
    std::cout << "Double buffer: " << value << std::endl;

    glfwMakeContextCurrent(window);

    int db = -1;
    glGetIntegerv(GL_DOUBLEBUFFER, &db);
    std::cout << "Double buffer: " << db << std::endl;


    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: "
            << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return -1;
    }
    std::cout << "GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;

   
  
    glewExperimental = GL_TRUE; 
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    
   
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);


    Fluidsim fluidSim(GRID_SIZE);
    g_fluid_Sim = &fluidSim;

    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT, GRID_SIZE);


    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);


    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        fluidSim.step();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer.draw(fluidSim.getDensityArray());

        glfwSwapBuffers(window);
    }
   
    glfwTerminate();
    return 0;
}
