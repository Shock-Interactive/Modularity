#include <iostream>
#include "Headers/Window/Window.h"

int main()
{
    Window window;
    GLFWwindow* editorWindow = window.makeWindow();

    if (!editorWindow) return -1;
    
    while (!glfwWindowShouldClose(editorWindow)) {
        glfwSwapBuffers(editorWindow);
        glfwPollEvents();
    }

    return 0;
}