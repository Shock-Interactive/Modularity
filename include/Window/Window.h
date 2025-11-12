#ifndef WINDOW_H
#define WINDOW_H
#include "../General/GlobalHeaders.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window {
public:
    GLFWwindow* makeWindow();
};

#endif