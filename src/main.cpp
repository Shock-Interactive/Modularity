#include <iostream>
#include "Headers/Window/Window.h"

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