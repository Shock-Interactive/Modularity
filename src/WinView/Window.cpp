#include "../Headers/Window/Window.h"

GLFWwindow* Window::makeWindow() {
#if defined(__linux__)
    setenv("XDG_SESSION_TYPE", "x11", 1);
#endif

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Modularity", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    
    return window;
}