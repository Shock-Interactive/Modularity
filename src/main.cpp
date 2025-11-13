#include <iostream>
#include <glad/glad.h>
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/backends/imgui_impl_glfw.h"
#include "ThirdParty/imgui/backends/imgui_impl_opengl3.h"
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

    // now thats a me error...
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    Shader shader("Shaders/vert.glsl", "Shaders/frag.glsl");

    // IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    ImGui_ImplGlfw_InitForOpenGL(editorWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330");


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

    GLint timeLoc  = glGetUniformLocation(shader.ID, "time");
    GLint uTimeLoc = glGetUniformLocation(shader.ID, "uTime");
    GLint speedLoc = glGetUniformLocation(shader.ID, "uSpeed");

    float spinSpeed = 1.0f;

    while (!glfwWindowShouldClose(editorWindow))
    {

        if (glfwGetWindowAttrib(editorWindow, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //Gooui part get it?
        ImGui::Button("Testing!");
        ImGui::SliderFloat("Spin Speed", &spinSpeed, 1.0f, 1000.0f);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        float t = static_cast<float>(glfwGetTime());

        glUniform1f(timeLoc,  t);
        glUniform1f(uTimeLoc, t);
        glUniform1f(speedLoc, spinSpeed);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(editorWindow);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}