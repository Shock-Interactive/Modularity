#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <glad/glad.h>
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/backends/imgui_impl_glfw.h"
#include "ThirdParty/imgui/backends/imgui_impl_opengl3.h"
#include "../include/Window/Window.h"
#include "../include/Shaders/Shader.h"
#include "../include/Textures/Texture.h"
#include "../include/Skybox/Skybox.h"
#include "ThirdParty/glm/gtc/matrix_transform.hpp"
#include "ThirdParty/glm/gtc/type_ptr.hpp"

constexpr float SENSITIVITY = 0.1f;
constexpr float CAMERA_SPEED = 2.5f;
constexpr float FOV = 45.0f;
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 100.0f;

// Hardcoded cube vertices for now; we should probably load from file sometime later lol.
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

// Simple Mesh class for modularity (encapsulates VAO/VBO)
class Mesh {
private:
    unsigned int VAO, VBO;

public:
    Mesh(const float* vertexData, size_t size) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, size, vertexData, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // TexCoord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~Mesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void draw() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
};

// Camera class (encapsulates state and logic)
class Camera {
public:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    float yaw = -90.0f;
    float pitch = 0.0f;
    float speed = CAMERA_SPEED;
    float lastX = 400.0f, lastY = 300.0f;
    bool firstMouse = true;

    void processMouse(double xpos, double ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = (xpos - lastX) * SENSITIVITY;
        float yoffset = (lastY - ypos) * SENSITIVITY;
        lastX = xpos;
        lastY = ypos;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(direction);
    }

    void processKeyboard(float deltaTime, GLFWwindow* window) {
        // Tunable constants (or move to class scope).
        const float CAMERA_SPEED = 5.0f;
        const float SPRINT_SPEED = 10.0f;
        const float ACCELERATION = 15.0f;

        // Update speed based on shift (sprint overrides base speed).
        float currentSpeed = CAMERA_SPEED;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            currentSpeed = SPRINT_SPEED;
        }

        glm::vec3 desiredDir(0.0f);
        bool isMoving = false;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            desiredDir += front;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            desiredDir -= front;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            desiredDir -= glm::normalize(glm::cross(front, up));
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            desiredDir += glm::normalize(glm::cross(front, up));
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            desiredDir -= up;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            desiredDir += up;
            isMoving = true;
        }

        // Set target velocity (zero if no keys pressed).
        glm::vec3 targetVelocity(0.0f);
        if (isMoving) {
            desiredDir = glm::normalize(desiredDir);
            targetVelocity = desiredDir * currentSpeed;
        }

        float smoothFactor = 1.0f - std::exp(-ACCELERATION * deltaTime);
        velocity = glm::mix(velocity, targetVelocity, smoothFactor);

        position += velocity * deltaTime;
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }
};

// Renderer class (handles FBO, resize, and basic rendering)
class Renderer {
private:
    unsigned int framebuffer = 0, viewportTexture = 0, rbo = 0;
    int currentWidth = 800, currentHeight = 600;
    Shader* shader = nullptr;
    Texture* texture1 = nullptr;
    Texture* texture2 = nullptr;
    Mesh* cube = nullptr;
    Skybox* skybox = nullptr;

public:
    Renderer() = default;
    ~Renderer() {
        delete shader;
        delete texture1;
        delete texture2;
        delete cube;
        delete skybox;
        if (framebuffer) glDeleteFramebuffers(1, &framebuffer);
        if (viewportTexture) glDeleteTextures(1, &viewportTexture);
        if (rbo) glDeleteRenderbuffers(1, &rbo);
    }

    void initialize() {
        shader = new Shader("Resources/Shaders/vert.glsl", "Resources/Shaders/frag.glsl");
        if (shader->ID == 0) {
            std::cerr << "Shader compilation failed!\n";
            delete shader;
            shader = nullptr;
            throw std::runtime_error("Shader error");
        }

        texture1 = new Texture("Resources/Textures/container.jpg");
        texture2 = new Texture("Resources/Textures/awesomeface.png");
        cube = new Mesh(vertices, sizeof(vertices));

        skybox = new Skybox();

        setupFBO();
        glEnable(GL_DEPTH_TEST);
    }

    void resize(int w, int h) {
        if (w <= 0 || h <= 0 || w == currentWidth && h == currentHeight) return;
        currentWidth = w;
        currentHeight = h;

        glBindTexture(GL_TEXTURE_2D, viewportTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, currentWidth, currentHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, currentWidth, currentHeight);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer incomplete after resize!\n";
        }
    }

    int getWidth() const { return currentWidth; }
    int getHeight() const { return currentHeight; }

    void beginRender(const glm::mat4& view, const glm::mat4& proj) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, currentWidth, currentHeight);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox->draw(glm::value_ptr(view), glm::value_ptr(proj));

        shader->use();
        shader->setMat4("view", view);
        shader->setMat4("projection", proj);
        texture1->Bind(GL_TEXTURE0);
        texture2->Bind(GL_TEXTURE1);
        shader->setInt("texture1", 0);
        shader->setInt("texture2", 1);
    }

    Skybox* getSkybox() { return skybox; }

    void renderCube(const glm::mat4& model) {
        shader->setMat4("model", model);
        cube->draw();
    }

    void endRender() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int getViewportTexture() const { return viewportTexture; }

private:
    void setupFBO() {
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glGenTextures(1, &viewportTexture);
        glBindTexture(GL_TEXTURE_2D, viewportTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, currentWidth, currentHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewportTexture, 0);

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, currentWidth, currentHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer setup failed!\n";
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

class ViewportController {
private:
    bool escPressed = false;
    bool viewportFocused = false;
    bool manuallyUnfocused = false;

public:
    bool isViewportFocused() const { return viewportFocused; }
    bool isManuallyUnfocused() const { return manuallyUnfocused; }

    void updateFocusFromImGui(bool imguiFocused) {
        if (!manuallyUnfocused) {
            viewportFocused = imguiFocused;
        }
    }

    void setFocused(bool focused) {
        viewportFocused = focused;
    }

    void clearManualUnfocus() {
        manuallyUnfocused = false;
    }

    void unfocusManually() {
        viewportFocused = false;
        manuallyUnfocused = true;
    }

    void update(GLFWwindow* window, bool& cursorLocked) {
        // ESC to unfocus
        bool escNow = (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
        if (escNow && !escPressed && isViewportFocused()) {
            unfocusManually();
            cursorLocked = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        escPressed = escNow;
    }
};

class Engine;
void window_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

class Engine {
private:
    Window window;
    GLFWwindow* editorWindow = nullptr;
    Renderer renderer;
    Camera camera;
    ViewportController viewportController;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    bool cursorLocked = false;
    int viewportWidth = 800;
    int viewportHeight = 600;

public:
    Engine() = default;

    bool init() {
        editorWindow = window.makeWindow();
        if (!editorWindow) return false;

        glfwSetWindowUserPointer(editorWindow, this);
        glfwSetWindowSizeCallback(editorWindow, window_size_callback);

        // Mouse callback using lambda to capture 'this'
        auto mouse_cb = [](GLFWwindow* window, double xpos, double ypos) {
            auto* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
            if (!engine) return;

            int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
            if (!engine->viewportController.isViewportFocused() || cursorMode != GLFW_CURSOR_DISABLED) {
                return;
            }

            engine->camera.processMouse(xpos, ypos);
        };
        glfwSetCursorPosCallback(editorWindow, mouse_cb);

        try {
            renderer.initialize();
        } catch (...) {
            return false;
        }

        setupImGui();
        return true;
    }

    void run() {
        while (!glfwWindowShouldClose(editorWindow)) {
            if (glfwGetWindowAttrib(editorWindow, GLFW_ICONIFIED)) {
                ImGui_ImplGlfw_Sleep(10);
                continue;
            }

            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Cap deltaTime for stability
            deltaTime = std::min(deltaTime, 1.0f / 30.0f);

            glfwPollEvents();  // Single poll now

            // Handle input
            viewportController.update(editorWindow, cursorLocked);

            // Safety check: Ensure cursor is unlocked if not focused (fixes potential timing issues)
            if (!viewportController.isViewportFocused() && cursorLocked) {
                cursorLocked = false;
                glfwSetInputMode(editorWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                camera.firstMouse = false;  // Reset to prevent stale state
                std::cout << "Force-unlocked cursor\n";  // Debug print (remove later)
            }

            // Camera keyboard input (only if focused and locked)
            if (viewportController.isViewportFocused() && cursorLocked) {
                camera.processKeyboard(deltaTime, editorWindow);
            }

            // ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Viewport UI
            ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
            ImVec2 vsize = ImGui::GetContentRegionAvail();
            if (vsize.x > 0 && vsize.y > 0) {
                viewportWidth = static_cast<int>(vsize.x);
                viewportHeight = static_cast<int>(vsize.y);
                renderer.resize(viewportWidth, viewportHeight);
            }
            unsigned int tex = renderer.getViewportTexture();
            ImGui::Image((void*)(intptr_t)tex, vsize, ImVec2(0, 1), ImVec2(1, 0));

            bool mouseOverImage = ImGui::IsItemHovered();
            bool windowFocused = ImGui::IsWindowFocused();

            // Update focus from ImGui (only if not manually unfocused)
            viewportController.updateFocusFromImGui(windowFocused);

            // Click to focus and lock
            if (mouseOverImage && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                viewportController.setFocused(true);
                viewportController.clearManualUnfocus();
                cursorLocked = true;
                glfwSetInputMode(editorWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                camera.firstMouse = true;
            }

            ImGui::End();

            // Inspector window for skybox time of day control
            ImGui::Begin("Inspector");
            if (renderer.getSkybox()) {
                float timeOfDay = renderer.getSkybox()->getTimeOfDay();
                ImGui::Text("Skybox Controls");
                ImGui::Separator();
                ImGui::SliderFloat("Time of Day", &timeOfDay, 0.0f, 1.0f, "%.2f");
                ImGui::Text("0.0 = Night");
                ImGui::Text("0.25 = Sunrise");
                ImGui::Text("0.5 = Day");
                ImGui::Text("0.75 = Sunset");
                ImGui::Text("1.0 = Midnight");
                renderer.getSkybox()->setTimeOfDay(timeOfDay);
            }
            ImGui::End();

            // Placeholder UI stuff
            //ImGui::Begin("Bank Account");
            //ImGui::Text("Balance: $0.00 (broke as fuck)");
            //ImGui::End();

            // Render scene to FBO now that size is known
            glm::mat4 model = glm::rotate(glm::mat4(1.0f), currentFrame * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
            glm::mat4 view = camera.getViewMatrix();
            float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
            if (aspect <= 0.0f) aspect = 1.0f;
            glm::mat4 proj = glm::perspective(glm::radians(FOV), aspect, NEAR_PLANE, FAR_PLANE);

            renderer.beginRender(view, proj);
            renderer.renderCube(model);
            renderer.endRender();

            // Main window render
            int displayW, displayH;
            glfwGetFramebufferSize(editorWindow, &displayW, &displayH);
            glViewport(0, 0, displayW, displayH);
            glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(editorWindow);
        }
    }

    void shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
    }

private:
    void setupImGui() {
        float mainScale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(mainScale);
        style.FontScaleDpi = mainScale;

        ImGui_ImplGlfw_InitForOpenGL(editorWindow, true);
        if (!ImGui_ImplOpenGL3_Init("#version 330")) {
            std::cerr << "ImGui OpenGL3 init failed!\n";
            throw std::runtime_error("ImGui error");
        }
    }
};

int main() {
    Engine engine;
    if (!engine.init()) {
        std::cerr << "Engine init failed!\n";
        return -1;
    }

    engine.run();
    engine.shutdown();
    return 0;
}