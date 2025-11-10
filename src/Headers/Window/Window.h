#ifndef WINDOW_H
#define WINDOW_H
#include "../General/GlobalHeaders.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
public:
    GLFWwindow* makeWindow();
};

#endif