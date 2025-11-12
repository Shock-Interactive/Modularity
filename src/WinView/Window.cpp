#include "../../include/Window/Window.h"

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
<<<<<<< HEAD
    
=======

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return nullptr;
    }

>>>>>>> 1ec3aa52144b306094f1119dfd3899de9664f7b8
    return window;
}