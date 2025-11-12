#include <iostream>
#include "../include/Window/Window.h"

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};  

int main()
{
    Window window;
    GLFWwindow* editorWindow = window.makeWindow();

    if (!editorWindow) return -1;
    
    // Main loop
    while (!glfwWindowShouldClose(editorWindow)) {
        glfwSwapBuffers(editorWindow);
        glfwPollEvents();
    }

    return 0;
}