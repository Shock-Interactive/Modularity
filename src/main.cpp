#include <iostream>
#include <glad/glad.h>
#include "../include/Window/Window.h"
#include "../include/Shaders/Shader.h"

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
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    Shader shader("Shaders/vert.glsl", "Shaders/frag.glsl");

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(editorWindow)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        float timeValue = glfwGetTime();
        int timeLoc = glGetUniformLocation(shader.ID, "time");
        glUniform1f(timeLoc, timeValue);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(editorWindow);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}