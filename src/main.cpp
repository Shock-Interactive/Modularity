#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <glad/glad.h>
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"
#include "ThirdParty/imgui/backends/imgui_impl_glfw.h"
#include "ThirdParty/imgui/backends/imgui_impl_opengl3.h"
#include "ThirdParty/ImGuizmo/ImGuizmo.h"
#include "ThirdParty/glm/gtc/matrix_transform.hpp"
#include "ThirdParty/glm/gtc/type_ptr.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "../include/ThirdParty/tiny_obj_loader.h"
#include "../include/Window/Window.h"
#include "../include/Shaders/Shader.h"
#include "../include/Textures/Texture.h"
#include "../include/Skybox/Skybox.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

namespace fs = std::filesystem;

constexpr float SENSITIVITY = 0.1f;
constexpr float CAMERA_SPEED = 2.5f;
constexpr float FOV = 45.0f;
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 100.0f;
constexpr float PI = 3.14159265359f;

// Replace the existing float vertices[] array (lines ~50-100) with this full 8-float version (pos + normal + texcoord)
float vertices[] = {
    // Back face (z = -0.5f)
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    // Front face (z = 0.5f)
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

    // Left face (x = -0.5f)
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    // Right face (x = 0.5f)
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    // Bottom face (y = -0.5f)
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    // Top face (y = 0.5f)
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};

enum class ObjectType {
    Cube,
    Sphere,
    Capsule,
    OBJMesh  // New type for loaded OBJ models
};

enum class ConsoleMessageType {
    Info,
    Warning,
    Error,
    Success
};

class SceneObject {
public:
    std::string name;
    ObjectType type;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    int id;
    int parentId = -1;
    std::vector<int> childIds;
    bool isExpanded = true;
    std::string meshPath;  // Path to OBJ file (for OBJMesh type)
    int meshId = -1;       // Index into loaded meshes cache

    SceneObject(const std::string& name, ObjectType type, int id)
        : name(name), type(type), position(0.0f), rotation(0.0f), scale(1.0f), id(id) {}
};

class FileBrowser {
public:
    fs::path currentPath;
    fs::path selectedFile;
    std::vector<fs::directory_entry> entries;
    bool needsRefresh = true;

    FileBrowser() {
        currentPath = fs::current_path();
    }

    void refresh() {
        entries.clear();
        try {
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                entries.push_back(entry);
            }
            std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
                if (a.is_directory() != b.is_directory()) {
                    return a.is_directory() > b.is_directory();
                }
                return a.path().filename().string() < b.path().filename().string();
            });
        } catch (...) {
        }
        needsRefresh = false;
    }

    void navigateUp() {
        if (currentPath.has_parent_path() && currentPath != currentPath.root_path()) {
            currentPath = currentPath.parent_path();
            needsRefresh = true;
        }
    }

    void navigateTo(const fs::path& path) {
        if (fs::is_directory(path)) {
            currentPath = path;
            needsRefresh = true;
        }
    }

    const char* getFileIcon(const fs::directory_entry& entry) const {
        if (entry.is_directory()) return "[D]";

        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".cpp" || ext == ".c" || ext == ".h" || ext == ".hpp") return "[C]";
        if (ext == ".glsl" || ext == ".vert" || ext == ".frag") return "[S]";
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") return "[I]";
        if (ext == ".obj" || ext == ".fbx" || ext == ".gltf") return "[M]";
        if (ext == ".txt" || ext == ".md") return "[T]";
        return "[F]";
    }
    
    bool isOBJFile(const fs::directory_entry& entry) const {
        if (entry.is_directory()) return false;
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".obj";
    }
};

// Replace the entire generateSphere() function (around line ~200) with this version that adds normals (now 8 floats per vertex)
std::vector<float> generateSphere(int segments = 32, int rings = 16) {
    std::vector<float> vertices;

    for (int ring = 0; ring <= rings; ring++) {
        float theta = ring * PI / rings;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int seg = 0; seg <= segments; seg++) {
            float phi = seg * 2.0f * PI / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            // Position
            vertices.push_back(x * 0.5f);
            vertices.push_back(y * 0.5f);
            vertices.push_back(z * 0.5f);

            // Normal (same as position for unit sphere)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Texcoord
            vertices.push_back((float)seg / segments);
            vertices.push_back((float)ring / rings);
        }
    }

    std::vector<float> triangulated;
    int stride = segments + 1;
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int current = ring * stride + seg;
            int next = current + stride;

            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[current * 8 + i]);
            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[next * 8 + i]);
            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[(current + 1) * 8 + i]);

            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[(current + 1) * 8 + i]);
            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[next * 8 + i]);
            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[(next + 1) * 8 + i]);
        }
    }

    return triangulated;
}

// Replace the entire generateCapsule() function (around line ~250) with this version that adds normals (now 8 floats per vertex)
std::vector<float> generateCapsule(int segments = 16, int rings = 8) {
    std::vector<float> vertices;
    float cylinderHeight = 0.5f;
    float radius = 0.25f;

    // Top hemisphere
    for (int ring = 0; ring <= rings / 2; ring++) {
        float theta = ring * PI / rings;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int seg = 0; seg <= segments; seg++) {
            float phi = seg * 2.0f * PI / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta * radius;
            float y = cosTheta * radius + cylinderHeight;
            float z = sinPhi * sinTheta * radius;

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal (normalized direction from center)
            glm::vec3 normal = glm::normalize(glm::vec3(x, y - cylinderHeight, z));
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // Texcoord
            vertices.push_back((float)seg / segments);
            vertices.push_back((float)ring / (rings / 2));
        }
    }

    // Cylinder body (top and bottom rings)
    for (int i = 0; i <= 1; i++) {
        float y = i == 0 ? cylinderHeight : -cylinderHeight;
        for (int seg = 0; seg <= segments; seg++) {
            float phi = seg * 2.0f * PI / segments;
            float x = cos(phi) * radius;
            float z = sin(phi) * radius;

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal (outward for cylinder)
            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // Texcoord
            vertices.push_back((float)seg / segments);
            vertices.push_back(0.5f);
        }
    }

    // Bottom hemisphere
    for (int ring = rings / 2; ring <= rings; ring++) {
        float theta = ring * PI / rings;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int seg = 0; seg <= segments; seg++) {
            float phi = seg * 2.0f * PI / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta * radius;
            float y = cosTheta * radius - cylinderHeight;
            float z = sinPhi * sinTheta * radius;

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal
            glm::vec3 normal = glm::normalize(glm::vec3(x, y + cylinderHeight, z));
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // Texcoord
            vertices.push_back((float)seg / segments);
            vertices.push_back((float)ring / rings);
        }
    }

    std::vector<float> triangulated;
    int stride = segments + 1;
    int totalRings = rings + 3;  // hemispheres + 3 for cylinder rings? Wait, adjust if needed

    for (int ring = 0; ring < totalRings - 1; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int current = ring * stride + seg;
            int next = current + stride;

            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[current * 8 + i]);
            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[next * 8 + i]);
            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[(current + 1) * 8 + i]);

            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[(current + 1) * 8 + i]);
            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[next * 8 + i]);
            for (int i = 0; i < 8; i++) triangulated.push_back(vertices[(next + 1) * 8 + i]);
        }
    }

    return triangulated;
}

class Mesh {
private:
    unsigned int VAO, VBO;
    int vertexCount;

public:
    Mesh(const float* vertexData, size_t dataSizeBytes) {
        vertexCount = dataSizeBytes / (8 * sizeof(float));

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, dataSizeBytes, vertexData, GL_STATIC_DRAW);

        // 0: Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // 1: Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // 2: TexCoord
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~Mesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void draw() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
    
    int getVertexCount() const { return vertexCount; }
};

class OBJLoader {
public:
    struct LoadedMesh {
        std::string path;
        std::unique_ptr<Mesh> mesh;
        std::string name;
        int vertexCount = 0;
        int faceCount = 0;
        bool hasNormals = false;
        bool hasTexCoords = false;
    };
    
private:
    std::vector<LoadedMesh> loadedMeshes;
    
public:
    // Load an OBJ file and return index into cache, or -1 on failure
    int loadOBJ(const std::string& filepath, std::string& errorMsg) {
        // Check if already loaded
        for (size_t i = 0; i < loadedMeshes.size(); i++) {
            if (loadedMeshes[i].path == filepath) {
                return static_cast<int>(i);
            }
        }
        
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        
        std::string baseDir = fs::path(filepath).parent_path().string();
        if (!baseDir.empty()) baseDir += "/";
        
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, 
                                    filepath.c_str(), baseDir.c_str());
        
        if (!warn.empty()) {
            errorMsg += "Warning: " + warn + "\n";
        }
        
        if (!err.empty()) {
            errorMsg += "Error: " + err + "\n";
        }
        
        if (!ret || shapes.empty()) {
            errorMsg += "Failed to load OBJ file: " + filepath;
            return -1;
        }
        
        // Convert to our vertex format (pos + uv)
        // Convert to our vertex format (pos + normal + uv)
        std::vector<float> vertices;
        bool hasNormalsInFile = !attrib.normals.empty();

        size_t totalIndices = 0;
        int faceCount = 0;
        for (const auto& shape : shapes) {
            totalIndices += shape.mesh.indices.size();
            faceCount += static_cast<int>(shape.mesh.num_face_vertices.size());
        }
        vertices.reserve(totalIndices * 8 / 3);  // rough estimate for triangulated verts

        for (const auto& shape : shapes) {
            size_t indexOffset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                int fv = shape.mesh.num_face_vertices[f];

                // Gather vertices for the face
                struct TempVertex {
                    glm::vec3 pos;
                    glm::vec2 uv;
                    glm::vec3 normal;
                    bool hasNormal = false;
                };
                std::vector<TempVertex> faceVerts;

                for (int v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];

                    TempVertex tv;
                    tv.pos.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    tv.pos.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    tv.pos.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                    if (idx.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                        tv.uv.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        tv.uv.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    } else {
                        tv.uv = glm::vec2(0.0f);
                    }

                    if (idx.normal_index >= 0 && hasNormalsInFile) {
                        tv.normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        tv.normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        tv.normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
                        tv.hasNormal = true;
                    }

                    faceVerts.push_back(tv);
                }

                // Compute face normal if no normals in file
                if (!hasNormalsInFile && fv >= 3) {
                    glm::vec3 v0 = faceVerts[0].pos;
                    glm::vec3 v1 = faceVerts[1].pos;
                    glm::vec3 v2 = faceVerts[2].pos;
                    glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

                    for (auto& tv : faceVerts) {
                        tv.normal = faceNormal;
                        tv.hasNormal = true;
                    }
                }

                // Triangulate fan
                for (int v = 1; v < fv - 1; v++) {
                    const TempVertex* tri[3] = { &faceVerts[0], &faceVerts[v], &faceVerts[v+1] };

                    for (int i = 0; i < 3; i++) {
                        // Pos
                        vertices.push_back(tri[i]->pos.x);
                        vertices.push_back(tri[i]->pos.y);
                        vertices.push_back(tri[i]->pos.z);

                        // Normal
                        vertices.push_back(tri[i]->normal.x);
                        vertices.push_back(tri[i]->normal.y);
                        vertices.push_back(tri[i]->normal.z);

                        // UV
                        vertices.push_back(tri[i]->uv.x);
                        vertices.push_back(tri[i]->uv.y);
                    }
                }

                indexOffset += fv;
            }
        }

        if (vertices.empty()) {
            errorMsg += "No vertices found in OBJ file";
            return -1;
        }

        // Create mesh
        LoadedMesh loaded;
        loaded.path = filepath;
        loaded.name = fs::path(filepath).stem().string();
        loaded.mesh = std::make_unique<Mesh>(vertices.data(), vertices.size() * sizeof(float));
        loaded.vertexCount = static_cast<int>(vertices.size() / 8);
        loaded.faceCount = faceCount;
        loaded.hasNormals = hasNormalsInFile;
        loaded.hasTexCoords = !attrib.texcoords.empty();

        loadedMeshes.push_back(std::move(loaded));
        return static_cast<int>(loadedMeshes.size() - 1);
    }
    
    // Get mesh by index
    Mesh* getMesh(int index) {
        if (index < 0 || index >= static_cast<int>(loadedMeshes.size())) {
            return nullptr;
        }
        return loadedMeshes[index].mesh.get();
    }
    
    // Get mesh info
    const LoadedMesh* getMeshInfo(int index) const {
        if (index < 0 || index >= static_cast<int>(loadedMeshes.size())) {
            return nullptr;
        }
        return &loadedMeshes[index];
    }
    
    // Get all loaded meshes (for UI display)
    const std::vector<LoadedMesh>& getAllMeshes() const {
        return loadedMeshes;
    }
    
    // Clear all loaded meshes
    void clear() {
        loadedMeshes.clear();
    }
    
    size_t getMeshCount() const { return loadedMeshes.size(); }
};

// Global OBJ loader instance
OBJLoader g_objLoader;

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
        const float CAMERA_SPEED = 5.0f;
        const float SPRINT_SPEED = 10.0f;
        const float ACCELERATION = 15.0f;

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
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glm::vec3 targetVelocity(0.0f);
        if (isMoving) {
            float length = glm::length(desiredDir);
            if (length > 0.0001f) {
                desiredDir = desiredDir / length;
                targetVelocity = desiredDir * currentSpeed;
            } else {
                targetVelocity = glm::vec3(0.0f);
            }
        }

        float smoothFactor = 1.0f - std::exp(-ACCELERATION * deltaTime);
        velocity = glm::mix(velocity, targetVelocity, smoothFactor);

        position += velocity * deltaTime;
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }
};

class ViewportController {
private:
    bool viewportFocused = false;
    bool manualUnfocus = false;

public:
    void updateFocusFromImGui(bool windowFocused) {
        if (!windowFocused && viewportFocused && !manualUnfocus) {
            viewportFocused = false;
        }
    }

    void setFocused(bool focused) {
        viewportFocused = focused;
    }

    bool isViewportFocused() const {
        return viewportFocused;
    }

    void clearManualUnfocus() {
        manualUnfocus = false;
    }

    void update(GLFWwindow* window, bool& cursorLocked) {
        if (viewportFocused && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            viewportFocused = false;
            manualUnfocus = true;
            cursorLocked = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
};

class Renderer {
private:
    unsigned int framebuffer = 0, viewportTexture = 0, rbo = 0;
    int currentWidth = 800, currentHeight = 600;
    Shader* shader = nullptr;
    Texture* texture1 = nullptr;
    Texture* texture2 = nullptr;
    Mesh* cubeMesh = nullptr;
    Mesh* sphereMesh = nullptr;
    Mesh* capsuleMesh = nullptr;
    Skybox* skybox = nullptr;

public:
    Renderer() = default;
    ~Renderer() {
        delete shader;
        delete texture1;
        delete texture2;
        delete cubeMesh;
        delete sphereMesh;
        delete capsuleMesh;
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

        cubeMesh = new Mesh(vertices, sizeof(vertices));

        auto sphereVerts = generateSphere();
        sphereMesh = new Mesh(sphereVerts.data(), sphereVerts.size() * sizeof(float));

        auto capsuleVerts = generateCapsule();
        capsuleMesh = new Mesh(capsuleVerts.data(), capsuleVerts.size() * sizeof(float));

        skybox = new Skybox();

        setupFBO();
        glEnable(GL_DEPTH_TEST);
    }

    void resize(int w, int h) {
        if (w <= 0 || h <= 0 || (w == currentWidth && h == currentHeight)) return;
        
        std::cout << "RESIZE TRIGGERED: " << w << "x" << h << std::endl;
        
        GLint currentFB;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFB);
        std::cout << "Framebuffer bound during resize: " << currentFB << " (should be " << framebuffer << ")" << std::endl;
        
        currentWidth = w;
        currentHeight = h;
        
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        
        glBindTexture(GL_TEXTURE_2D, viewportTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, currentWidth, currentHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, currentWidth, currentHeight);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer incomplete after resize!\n";
        }
        
        std::cout << "Resize complete" << std::endl;
    }

    int getWidth() const { return currentWidth; }
    int getHeight() const { return currentHeight; }

    void beginRender(const glm::mat4& view, const glm::mat4& proj) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, currentWidth, currentHeight);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->use();
        shader->setMat4("view", view);
        shader->setMat4("projection", proj);
        texture1->Bind(GL_TEXTURE0);
        texture2->Bind(GL_TEXTURE1);
        shader->setInt("texture1", 0);
        shader->setInt("texture2", 1);
    }

    void renderSkybox(const glm::mat4& view, const glm::mat4& proj) {
        if (skybox) {
            GLint currentFB;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFB);
            
            glDepthFunc(GL_LEQUAL);
            skybox->draw(glm::value_ptr(view), glm::value_ptr(proj));
            glDepthFunc(GL_LESS);
            
            GLint afterFB;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &afterFB);
            
            if (currentFB != afterFB) {
                std::cerr << "WARNING: Framebuffer changed during skybox render! " 
                        << currentFB << " -> " << afterFB << std::endl;
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            }
            
            shader->use();
            shader->setMat4("view", view);
            shader->setMat4("projection", proj);
            shader->use();
        }
    }

    Skybox* getSkybox() { return skybox; }

    void renderObject(const SceneObject& obj) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, obj.position);
        model = glm::rotate(model, glm::radians(obj.rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(obj.rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, obj.scale);

        shader->setMat4("model", model);

        switch (obj.type) {
            case ObjectType::Cube:
                cubeMesh->draw();
                break;
            case ObjectType::Sphere:
                sphereMesh->draw();
                break;
            case ObjectType::Capsule:
                capsuleMesh->draw();
                break;
            case ObjectType::OBJMesh:
                // Draw loaded OBJ mesh
                if (obj.meshId >= 0) {
                    Mesh* objMesh = g_objLoader.getMesh(obj.meshId);
                    if (objMesh) {
                        objMesh->draw();
                    }
                }
                break;
        }
    }

    void renderScene(const Camera& camera, const std::vector<SceneObject>& sceneObjects) {
        shader->use();
        shader->setMat4("view", camera.getViewMatrix());
        shader->setMat4("projection", glm::perspective(glm::radians(FOV), (float)currentWidth / (float)currentHeight, NEAR_PLANE, FAR_PLANE));
        shader->setVec3("lightPos", glm::vec3(4.0f, 6.0f, 4.0f));  // Slightly higher and farther
        shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        shader->setFloat("ambientStrength", 0.25f);
        shader->setFloat("specularStrength", 0.8f);
        shader->setFloat("shininess", 64.0f);
        shader->setFloat("mixAmount", 0.3f);

        texture1->Bind(0);
        texture2->Bind(1);

        for (const auto& obj : sceneObjects) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, obj.position);
            model = glm::rotate(model, glm::radians(obj.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(obj.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, obj.scale);

            shader->setMat4("model", model);

            Mesh* meshToDraw = nullptr;
            if (obj.type == ObjectType::Cube) meshToDraw = cubeMesh;
            else if (obj.type == ObjectType::Sphere) meshToDraw = sphereMesh;
            else if (obj.type == ObjectType::Capsule) meshToDraw = capsuleMesh;
            else if (obj.type == ObjectType::OBJMesh && obj.meshId != -1) {
                meshToDraw = g_objLoader.getMesh(obj.meshId);
            }

            if (meshToDraw) {
                meshToDraw->draw();
            }
        }

        // Skybox last
        if (skybox) {
            glm::mat4 view = camera.getViewMatrix();
            glm::mat4 proj = glm::perspective(glm::radians(FOV), 
                                            (float)currentWidth / currentHeight, 
                                            NEAR_PLANE, FAR_PLANE);

            skybox->draw(glm::value_ptr(view), glm::value_ptr(proj));
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

struct RecentProject {
    std::string name;
    std::string path;
    std::string lastOpened;
};

class Project {
public:
    std::string name;
    fs::path projectPath;
    fs::path scenesPath;
    fs::path assetsPath;
    fs::path scriptsPath;
    std::string currentSceneName;
    bool isLoaded = false;
    bool hasUnsavedChanges = false;

    Project() = default;

    Project(const std::string& projectName, const fs::path& basePath)
        : name(projectName) {
        projectPath = basePath / projectName;
        scenesPath = projectPath / "Scenes";
        assetsPath = projectPath / "Assets";
        scriptsPath = projectPath / "Scripts";
    }

    bool create() {
        try {
            fs::create_directories(projectPath);
            fs::create_directories(scenesPath);
            fs::create_directories(assetsPath);
            fs::create_directories(assetsPath / "Textures");
            fs::create_directories(assetsPath / "Models");
            fs::create_directories(assetsPath / "Shaders");
            fs::create_directories(scriptsPath);

            saveProjectFile();

            currentSceneName = "Main";
            isLoaded = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to create project: " << e.what() << std::endl;
            return false;
        }
    }

    bool load(const fs::path& projectFilePath) {
        try {
            projectPath = projectFilePath.parent_path();
            scenesPath = projectPath / "Scenes";
            assetsPath = projectPath / "Assets";
            scriptsPath = projectPath / "Scripts";

            std::ifstream file(projectFilePath);
            if (!file.is_open()) return false;

            std::string line;
            while (std::getline(file, line)) {
                if (line.find("name=") == 0) {
                    name = line.substr(5);
                } else if (line.find("lastScene=") == 0) {
                    currentSceneName = line.substr(10);
                }
            }
            file.close();

            if (currentSceneName.empty()) {
                currentSceneName = "Main";
            }

            isLoaded = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load project: " << e.what() << std::endl;
            return false;
        }
    }

    void saveProjectFile() const {
        std::ofstream file(projectPath / "project.modu");
        file << "name=" << name << "\n";
        file << "lastScene=" << currentSceneName << "\n";
        file.close();
    }

    std::vector<std::string> getSceneList() const {
        std::vector<std::string> scenes;
        try {
            for (const auto& entry : fs::directory_iterator(scenesPath)) {
                if (entry.path().extension() == ".scene") {
                    scenes.push_back(entry.path().stem().string());
                }
            }
        } catch (...) {}
        return scenes;
    }

    fs::path getSceneFilePath(const std::string& sceneName) const {
        return scenesPath / (sceneName + ".scene");
    }
};

class ProjectManager {
public:
    std::vector<RecentProject> recentProjects;
    fs::path appDataPath;
    char newProjectName[128] = "";
    char newProjectLocation[512] = "";
    char openProjectPath[512] = "";
    bool showNewProjectDialog = false;
    bool showOpenProjectDialog = false;
    std::string errorMessage;
    Project currentProject;

    ProjectManager() {
        #ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        if (appdata) {
            appDataPath = fs::path(appdata) / ".Modularity";
        } else {
            appDataPath = fs::current_path() / "AppData";
        }
        #else
        const char* home = std::getenv("HOME");
        if (home) {
            appDataPath = fs::path(home) / ".Modularity";
        } else {
            appDataPath = fs::current_path() / ".Modularity";
        }
        #endif

        fs::create_directories(appDataPath);
        loadRecentProjects();

        std::string defaultPath = (fs::current_path() / "Projects").string();
        strncpy(newProjectLocation, defaultPath.c_str(), sizeof(newProjectLocation) - 1);
    }

    void loadRecentProjects() {
        recentProjects.clear();
        fs::path recentFile = appDataPath / "recent_projects.txt";

        if (!fs::exists(recentFile)) return;

        std::ifstream file(recentFile);
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            RecentProject rp;
            size_t pos1 = line.find('|');
            size_t pos2 = line.find('|', pos1 + 1);

            if (pos1 != std::string::npos && pos2 != std::string::npos) {
                rp.name = line.substr(0, pos1);
                rp.path = line.substr(pos1 + 1, pos2 - pos1 - 1);
                rp.lastOpened = line.substr(pos2 + 1);

                if (fs::exists(rp.path)) {
                    recentProjects.push_back(rp);
                }
            }
        }
        file.close();
    }

    void saveRecentProjects() {
        fs::path recentFile = appDataPath / "recent_projects.txt";
        std::ofstream file(recentFile);

        for (const auto& rp : recentProjects) {
            file << rp.name << "|" << rp.path << "|" << rp.lastOpened << "\n";
        }
        file.close();
    }

    void addToRecentProjects(const std::string& name, const std::string& path) {
        recentProjects.erase(
            std::remove_if(recentProjects.begin(), recentProjects.end(),
                [&path](const RecentProject& rp) { return rp.path == path; }),
            recentProjects.end()
        );

        std::time_t now = std::time(nullptr);
        char timeStr[64];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", std::localtime(&now));

        RecentProject rp;
        rp.name = name;
        rp.path = path;
        rp.lastOpened = timeStr;
        recentProjects.insert(recentProjects.begin(), rp);

        if (recentProjects.size() > 10) {
            recentProjects.resize(10);
        }

        saveRecentProjects();
    }

    bool loadProject(const std::string& projectPath) {
        fs::path path(projectPath);
        if (fs::is_directory(path)) {
            path = path / "project.modu";
        }

        if (!fs::exists(path)) {
            errorMessage = "Project file not found.";
            return false;
        }

        Project loadedProject;
        if (loadedProject.load(path)) {
            currentProject = loadedProject;
            addToRecentProjects(currentProject.name, path.string());
            return true;
        } else {
            errorMessage = "Failed to load project.";
            return false;
        }
    }
};

class SceneSerializer {
public:
    static bool saveScene(const fs::path& filePath,
                         const std::vector<SceneObject>& objects,
                         int nextId) {
        try {
            std::ofstream file(filePath);
            if (!file.is_open()) return false;

            file << "# Scene File\n";
            file << "version=2\n";  // Bumped version for new format
            file << "nextId=" << nextId << "\n";
            file << "objectCount=" << objects.size() << "\n";
            file << "\n";

            for (const auto& obj : objects) {
                file << "[Object]\n";
                file << "id=" << obj.id << "\n";
                file << "name=" << obj.name << "\n";
                file << "type=" << static_cast<int>(obj.type) << "\n";
                file << "parentId=" << obj.parentId << "\n";
                file << "position=" << obj.position.x << "," << obj.position.y << "," << obj.position.z << "\n";
                file << "rotation=" << obj.rotation.x << "," << obj.rotation.y << "," << obj.rotation.z << "\n";
                file << "scale=" << obj.scale.x << "," << obj.scale.y << "," << obj.scale.z << "\n";
                
                // Save mesh path for OBJ meshes
                if (obj.type == ObjectType::OBJMesh && !obj.meshPath.empty()) {
                    file << "meshPath=" << obj.meshPath << "\n";
                }

                file << "children=";
                for (size_t i = 0; i < obj.childIds.size(); i++) {
                    if (i > 0) file << ",";
                    file << obj.childIds[i];
                }
                file << "\n\n";
            }

            file.close();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to save scene: " << e.what() << std::endl;
            return false;
        }
    }

    static bool loadScene(const fs::path& filePath,
                         std::vector<SceneObject>& objects,
                         int& nextId) {
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) return false;

            objects.clear();
            std::string line;
            SceneObject* currentObj = nullptr;

            while (std::getline(file, line)) {
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);

                if (line.empty() || line[0] == '#') continue;

                if (line == "[Object]") {
                    objects.push_back(SceneObject("", ObjectType::Cube, 0));
                    currentObj = &objects.back();
                    continue;
                }

                size_t eqPos = line.find('=');
                if (eqPos == std::string::npos) continue;

                std::string key = line.substr(0, eqPos);
                std::string value = line.substr(eqPos + 1);

                if (key == "nextId") {
                    nextId = std::stoi(value);
                } else if (currentObj) {
                    if (key == "id") {
                        currentObj->id = std::stoi(value);
                    } else if (key == "name") {
                        currentObj->name = value;
                    } else if (key == "type") {
                        currentObj->type = static_cast<ObjectType>(std::stoi(value));
                    } else if (key == "parentId") {
                        currentObj->parentId = std::stoi(value);
                    } else if (key == "position") {
                        sscanf(value.c_str(), "%f,%f,%f",
                               &currentObj->position.x,
                               &currentObj->position.y,
                               &currentObj->position.z);
                    } else if (key == "rotation") {
                        sscanf(value.c_str(), "%f,%f,%f",
                               &currentObj->rotation.x,
                               &currentObj->rotation.y,
                               &currentObj->rotation.z);
                    } else if (key == "scale") {
                        sscanf(value.c_str(), "%f,%f,%f",
                               &currentObj->scale.x,
                               &currentObj->scale.y,
                               &currentObj->scale.z);
                    } else if (key == "meshPath") {
                        currentObj->meshPath = value;
                        // Reload the mesh
                        if (!value.empty() && currentObj->type == ObjectType::OBJMesh) {
                            std::string err;
                            currentObj->meshId = g_objLoader.loadOBJ(value, err);
                        }
                    } else if (key == "children" && !value.empty()) {
                        std::stringstream ss(value);
                        std::string item;
                        while (std::getline(ss, item, ',')) {
                            if (!item.empty()) {
                                currentObj->childIds.push_back(std::stoi(item));
                            }
                        }
                    }
                }
            }

            file.close();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load scene: " << e.what() << std::endl;
            return false;
        }
    }
};

void window_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

class Engine;

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

    std::vector<SceneObject> sceneObjects;
    int selectedObjectId = -1;
    int nextObjectId = 0;

    FileBrowser fileBrowser;
    bool viewportFullscreen = false;
    bool showHierarchy = true;
    bool showInspector = true;
    bool showFileBrowser = true;
    bool showConsole = true;
    bool showProjectBrowser = true;
    bool firstFrame = true;
    std::vector<std::string> consoleLog;
    int draggedObjectId = -1;

    ProjectManager projectManager;
    bool showLauncher = true;
    bool showNewSceneDialog = false;
    bool showSaveSceneAsDialog = false;
    char newSceneName[128] = "";
    char saveSceneAsName[128] = "";
    bool rendererInitialized = false;
    
    bool showImportOBJDialog = false;
    std::string pendingOBJPath;
    char importOBJName[128] = "";

public:
    Engine() = default;

    bool init() {
        editorWindow = window.makeWindow();
        if (!editorWindow) return false;

        glfwSetWindowUserPointer(editorWindow, this);
        glfwSetWindowSizeCallback(editorWindow, window_size_callback);

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

        setupImGui();
        logToConsole("Engine initialized - Waiting for project selection");
        return true;
    }

    bool initRenderer() {
        if (rendererInitialized) return true;

        try {
            renderer.initialize();
            rendererInitialized = true;
            return true;
        } catch (...) {
            return false;
        }
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

            deltaTime = std::min(deltaTime, 1.0f / 30.0f);

            glfwPollEvents();

            if (!showLauncher) {
                handleKeyboardShortcuts();
            }

            viewportController.update(editorWindow, cursorLocked);

            if (!viewportController.isViewportFocused() && cursorLocked) {
                cursorLocked = false;
                glfwSetInputMode(editorWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                camera.firstMouse = true;
            }

            if (viewportController.isViewportFocused() && cursorLocked) {
                camera.processKeyboard(deltaTime, editorWindow);
            }


            if (!showLauncher && projectManager.currentProject.isLoaded && rendererInitialized) {
                glm::mat4 view = camera.getViewMatrix();
                float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
                if (aspect <= 0.0f) aspect = 1.0f;
                glm::mat4 proj = glm::perspective(glm::radians(FOV), aspect, NEAR_PLANE, FAR_PLANE);

                renderer.beginRender(view, proj);
                
                for (const auto& obj : sceneObjects) {
                    renderer.renderObject(obj);
                }

                renderer.renderSkybox(view, proj);
                
                renderer.endRender();
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (showLauncher) {
                renderLauncher();
            } else {
                setupDockspace();
                renderMainMenuBar();

                if (!viewportFullscreen) {
                    if (showHierarchy) renderHierarchyPanel();
                    if (showInspector) renderInspectorPanel();
                    if (showFileBrowser) renderFileBrowserPanel();
                    if (showConsole) renderConsolePanel();
                    if (showProjectBrowser) renderProjectBrowserPanel();
                }

                renderViewport();
                renderDialogs();
            }

            int displayW, displayH;
            glfwGetFramebufferSize(editorWindow, &displayW, &displayH);
            glViewport(0, 0, displayW, displayH);
            glClearColor(0.1f, 0.1f, 0.12f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }

            glfwSwapBuffers(editorWindow);
            firstFrame = false;
        }
    }

    void shutdown() {
        if (projectManager.currentProject.isLoaded && projectManager.currentProject.hasUnsavedChanges) {
            saveCurrentScene();
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
    }

private:
    void importOBJToScene(const std::string& filepath, const std::string& objectName) {
        std::string errorMsg;
        int meshId = g_objLoader.loadOBJ(filepath, errorMsg);
        
        if (meshId < 0) {
            addConsoleMessage("Failed to load OBJ: " + errorMsg, ConsoleMessageType::Error);
            return;
        }
        
        // Create scene object
        int id = nextObjectId++;
        std::string name = objectName.empty() ? fs::path(filepath).stem().string() : objectName;
        
        SceneObject obj(name, ObjectType::OBJMesh, id);
        obj.meshPath = filepath;
        obj.meshId = meshId;
        
        sceneObjects.push_back(obj);
        selectedObjectId = id;
        
        if (projectManager.currentProject.isLoaded) {
            projectManager.currentProject.hasUnsavedChanges = true;
        }
        
        const auto* meshInfo = g_objLoader.getMeshInfo(meshId);
        if (meshInfo) {
            addConsoleMessage("Imported OBJ: " + name + " (" + 
                            std::to_string(meshInfo->vertexCount) + " vertices, " +
                            std::to_string(meshInfo->faceCount) + " faces)", 
                            ConsoleMessageType::Success);
        } else {
            addConsoleMessage("Imported OBJ: " + name, ConsoleMessageType::Success);
        }
    }

    void handleKeyboardShortcuts() {
        static bool f11Pressed = false;
        if (glfwGetKey(editorWindow, GLFW_KEY_F11) == GLFW_PRESS && !f11Pressed) {
            viewportFullscreen = !viewportFullscreen;
            f11Pressed = true;
        }
        if (glfwGetKey(editorWindow, GLFW_KEY_F11) == GLFW_RELEASE) {
            f11Pressed = false;
        }

        static bool ctrlSPressed = false;
        bool ctrlDown = glfwGetKey(editorWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                       glfwGetKey(editorWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

        if (ctrlDown && glfwGetKey(editorWindow, GLFW_KEY_S) == GLFW_PRESS && !ctrlSPressed) {
            if (projectManager.currentProject.isLoaded) {
                saveCurrentScene();
            }
            ctrlSPressed = true;
        }
        if (glfwGetKey(editorWindow, GLFW_KEY_S) == GLFW_RELEASE) {
            ctrlSPressed = false;
        }

        static bool ctrlNPressed = false;
        if (ctrlDown && glfwGetKey(editorWindow, GLFW_KEY_N) == GLFW_PRESS && !ctrlNPressed) {
            if (projectManager.currentProject.isLoaded) {
                showNewSceneDialog = true;
                memset(newSceneName, 0, sizeof(newSceneName));
            }
            ctrlNPressed = true;
        }
        if (glfwGetKey(editorWindow, GLFW_KEY_N) == GLFW_RELEASE) {
            ctrlNPressed = false;
        }
    }

    void OpenProjectPath(const std::string& path) {
        if (projectManager.loadProject(path)) {
            if (!initRenderer()) {
                addConsoleMessage("Error: Failed to initialize renderer!", ConsoleMessageType::Error);
            } else {
                showLauncher = false;
                loadRecentScenes();
                addConsoleMessage("Opened project: " + projectManager.currentProject.name, ConsoleMessageType::Info);
            }
        } else {
            addConsoleMessage("Error opening project: " + projectManager.errorMessage, ConsoleMessageType::Error);
        }
    }

    void renderLauncher() {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 displaySize = io.DisplaySize;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.09f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(displaySize);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoMove     |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoDocking  |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        if (ImGui::Begin("Launcher", nullptr, flags))
        {
            float leftPanelWidth = 280.0f;

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.07f, 1.0f));
            ImGui::BeginChild("LauncherLeft", ImVec2(leftPanelWidth, 0), true);
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.45f, 0.72f, 0.95f, 1.0f), "MODULARITY");
            ImGui::TextDisabled("Game Engine");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.78f, 1.0f), "GET STARTED");
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.38f, 0.55f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.48f, 0.68f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.42f, 0.60f, 1.0f));

            if (ImGui::Button("New Project", ImVec2(-1, 36.0f)))
            {
                projectManager.showNewProjectDialog = true;
                projectManager.errorMessage.clear();
                std::memset(projectManager.newProjectName, 0, sizeof(projectManager.newProjectName));

                #ifdef _WIN32
                char documentsPath[MAX_PATH];
                SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, documentsPath);
                std::strcpy(projectManager.newProjectLocation, documentsPath);
                std::strcat(projectManager.newProjectLocation, "\\ModularityProjects");
                #else
                const char* home = std::getenv("HOME");
                if (home)
                {
                    std::strcpy(projectManager.newProjectLocation, home);
                    std::strcat(projectManager.newProjectLocation, "/ModularityProjects");
                }
                #endif
            }

            ImGui::Spacing();

            if (ImGui::Button("Open Project", ImVec2(-1, 36.0f)))
            {
                projectManager.showOpenProjectDialog = true;
                projectManager.errorMessage.clear();
            }

            ImGui::PopStyleColor(3);

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.78f, 1.0f), "QUICK ACTIONS");
            ImGui::Spacing();

            if (ImGui::Button("Documentation", ImVec2(-1, 30.0f)))
            {
                #ifdef _WIN32
                system("start https://github.com");
                #endif
            }

            if (ImGui::Button("Exit", ImVec2(-1, 30.0f)))
            {
                glfwSetWindowShouldClose(editorWindow, GLFW_TRUE);
            }

            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.11f, 1.0f));
            ImGui::BeginChild("LauncherRight", ImVec2(0, 0), true);
            ImGui::PopStyleColor();

            ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.78f, 1.0f), "RECENT PROJECTS");
            ImGui::Spacing();

            if (projectManager.recentProjects.empty())
            {
                ImGui::Spacing();
                ImGui::TextDisabled("No recent projects");
                ImGui::TextDisabled("Create a new project to get started!");
            }
            else
            {
                for (size_t i = 0; i < projectManager.recentProjects.size(); ++i)
                {
                    const auto& rp = projectManager.recentProjects[i];
                    ImGui::PushID(static_cast<int>(i));

                    char label[512];
                    std::snprintf(label, sizeof(label), "%s\n%s",
                                rp.name.c_str(), rp.path.c_str());

                    ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.20f, 0.30f, 0.45f, 0.40f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25f, 0.38f, 0.55f, 0.70f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.20f, 0.35f, 0.60f, 0.90f));

                    bool selected = ImGui::Selectable(
                        label,
                        false,
                        ImGuiSelectableFlags_AllowDoubleClick,
                        ImVec2(0.0f, 48.0f)
                    );

                    ImGui::PopStyleColor(3);

                    if (selected || ImGui::IsItemClicked(ImGuiMouseButton_Left))
                    {
                        OpenProjectPath(rp.path);
                    }

                    if (ImGui::BeginPopupContextItem("RecentProjectContext"))
                    {
                        if (ImGui::MenuItem("Open"))
                        {
                            OpenProjectPath(rp.path);
                        }

                        if (ImGui::MenuItem("Remove from Recent"))
                        {
                            projectManager.recentProjects.erase(
                                projectManager.recentProjects.begin() + i
                            );
                            projectManager.saveRecentProjects();
                            ImGui::EndPopup();
                            ImGui::PopID();
                            break;
                        }

                        ImGui::EndPopup();
                    }

                    ImGui::PopID();
                    ImGui::Spacing();
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextDisabled("Modularity Engine - Version 1.0.1");

            ImGui::EndChild();
        }

        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);

        if (projectManager.showNewProjectDialog)
            renderNewProjectDialog();
        if (projectManager.showOpenProjectDialog)
            renderOpenProjectDialog();
    }

    void renderNewProjectDialog() {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 250), ImGuiCond_Appearing);

        if (ImGui::Begin("New Project", &projectManager.showNewProjectDialog,
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking)) {

            ImGui::Text("Project Name:");
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##ProjectName", projectManager.newProjectName,
                           sizeof(projectManager.newProjectName));

            ImGui::Spacing();

            ImGui::Text("Location:");
            ImGui::SetNextItemWidth(-70);
            ImGui::InputText("##Location", projectManager.newProjectLocation,
                           sizeof(projectManager.newProjectLocation));
            ImGui::SameLine();
            if (ImGui::Button("Browse")) {
            }

            ImGui::Spacing();

            if (strlen(projectManager.newProjectName) > 0) {
                fs::path previewPath = fs::path(projectManager.newProjectLocation) /
                                      projectManager.newProjectName;
                ImGui::TextDisabled("Project will be created at:");
                ImGui::TextWrapped("%s", previewPath.string().c_str());
            }

            if (!projectManager.errorMessage.empty()) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s",
                                  projectManager.errorMessage.c_str());
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float buttonWidth = 100;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth * 2 - 20);

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                projectManager.showNewProjectDialog = false;
                memset(projectManager.newProjectName, 0, sizeof(projectManager.newProjectName));
            }

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.4f, 1.0f));
            if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
                if (strlen(projectManager.newProjectName) == 0) {
                    projectManager.errorMessage = "Please enter a project name";
                } else if (strlen(projectManager.newProjectLocation) == 0) {
                    projectManager.errorMessage = "Please specify a location";
                } else {
                    createNewProject(projectManager.newProjectName,
                                   projectManager.newProjectLocation);
                    projectManager.showNewProjectDialog = false;
                }
            }
            ImGui::PopStyleColor(2);
        }
        ImGui::End();
    }

    void renderOpenProjectDialog() {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 180), ImGuiCond_Appearing);

        if (ImGui::Begin("Open Project", &projectManager.showOpenProjectDialog,
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking)) {

            ImGui::Text("Project File Path (.modu):");
            ImGui::SetNextItemWidth(-70);
            ImGui::InputText("##OpenPath", projectManager.openProjectPath,
                           sizeof(projectManager.openProjectPath));
            ImGui::SameLine();
            if (ImGui::Button("Browse")) {
            }

            ImGui::TextDisabled("Select a project.modu file");

            if (!projectManager.errorMessage.empty()) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s",
                                  projectManager.errorMessage.c_str());
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float buttonWidth = 100;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth * 2 - 20);

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                projectManager.showOpenProjectDialog = false;
                memset(projectManager.openProjectPath, 0, sizeof(projectManager.openProjectPath));
            }

            ImGui::SameLine();

            if (ImGui::Button("Open", ImVec2(buttonWidth, 0))) {
                if (strlen(projectManager.openProjectPath) == 0) {
                    projectManager.errorMessage = "Please enter a project path";
                } else {
                    if (projectManager.loadProject(projectManager.openProjectPath)) {
                        if (!initRenderer()) {
                            addConsoleMessage("Error: Failed to initialize renderer!", ConsoleMessageType::Error);
                        } else {
                            showLauncher = false;
                            loadRecentScenes();
                            addConsoleMessage("Opened project: " + projectManager.currentProject.name, ConsoleMessageType::Info);
                        }
                    } else {
                        addConsoleMessage("Error opening project: " + projectManager.errorMessage, ConsoleMessageType::Error);
                    }
                }
            }
        }
        ImGui::End();
    }

    void createNewProject(const char* name, const char* location) {
        fs::path basePath(location);
        fs::create_directories(basePath);

        Project newProject(name, basePath);
        if (newProject.create()) {
            projectManager.currentProject = newProject;
            projectManager.addToRecentProjects(name,
                                              (newProject.projectPath / "project.modu").string());

            if (!initRenderer()) {
                logToConsole("Error: Failed to initialize renderer!");
                return;
            }

            sceneObjects.clear();
            selectedObjectId = -1;
            nextObjectId = 0;

            addObject(ObjectType::Cube, "Cube");

            fileBrowser.currentPath = projectManager.currentProject.assetsPath;
            fileBrowser.needsRefresh = true;

            showLauncher = false;
            firstFrame = true;

            addConsoleMessage("Created new project: " + std::string(name), ConsoleMessageType::Success);
            addConsoleMessage("Project location: " + newProject.projectPath.string(), ConsoleMessageType::Info);

            saveCurrentScene();
        } else {
            projectManager.errorMessage = "Failed to create project directory";
        }
    }

    void loadRecentScenes() {
        sceneObjects.clear();
        selectedObjectId = -1;
        nextObjectId = 0;

        fs::path scenePath = projectManager.currentProject.getSceneFilePath(projectManager.currentProject.currentSceneName);
        if (fs::exists(scenePath)) {
            if (SceneSerializer::loadScene(scenePath, sceneObjects, nextObjectId)) {
                addConsoleMessage("Loaded scene: " + projectManager.currentProject.currentSceneName, ConsoleMessageType::Success);
            } else {
                addConsoleMessage("Warning: Failed to load scene, starting fresh", ConsoleMessageType::Warning);
                addObject(ObjectType::Cube, "Cube");
            }
        } else {
            addConsoleMessage("Default scene not found, starting with a new scene.", ConsoleMessageType::Info);
            addObject(ObjectType::Cube, "Cube");
        }

        fileBrowser.currentPath = projectManager.currentProject.assetsPath;
        fileBrowser.needsRefresh = true;
    }

    void saveCurrentScene() {
        if (!projectManager.currentProject.isLoaded) return;

        fs::path scenePath = projectManager.currentProject.getSceneFilePath(projectManager.currentProject.currentSceneName);
        if (SceneSerializer::saveScene(scenePath, sceneObjects, nextObjectId)) {
            projectManager.currentProject.hasUnsavedChanges = false;
            projectManager.currentProject.saveProjectFile();
            addConsoleMessage("Saved scene: " + projectManager.currentProject.currentSceneName, ConsoleMessageType::Success);
        } else {
            addConsoleMessage("Error: Failed to save scene!", ConsoleMessageType::Error);
        }
    }

    void loadScene(const std::string& sceneName) {
        if (!projectManager.currentProject.isLoaded) return;

        if (projectManager.currentProject.hasUnsavedChanges) {
            saveCurrentScene();
        }

        fs::path scenePath = projectManager.currentProject.getSceneFilePath(sceneName);
        if (SceneSerializer::loadScene(scenePath, sceneObjects, nextObjectId)) {
            projectManager.currentProject.currentSceneName = sceneName;
            projectManager.currentProject.hasUnsavedChanges = false;
            projectManager.currentProject.saveProjectFile();
            selectedObjectId = -1;
            addConsoleMessage("Loaded scene: " + sceneName, ConsoleMessageType::Success);
        } else {
            addConsoleMessage("Error: Failed to load scene: " + sceneName, ConsoleMessageType::Error);
        }
    }

    void createNewScene(const std::string& sceneName) {
        if (!projectManager.currentProject.isLoaded || sceneName.empty()) return;

        if (projectManager.currentProject.hasUnsavedChanges) {
            saveCurrentScene();
        }

        sceneObjects.clear();
        selectedObjectId = -1;
        nextObjectId = 0;

        projectManager.currentProject.currentSceneName = sceneName;
        projectManager.currentProject.hasUnsavedChanges = true;

        addObject(ObjectType::Cube, "Cube");

        saveCurrentScene();

        addConsoleMessage("Created new scene: " + sceneName, ConsoleMessageType::Success);
    }

    void renderDialogs() {
        if (showNewSceneDialog) {
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(350, 130), ImGuiCond_Appearing);

            if (ImGui::Begin("New Scene", &showNewSceneDialog,
                            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking)) {
                ImGui::Text("Scene Name:");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##NewSceneName", newSceneName, sizeof(newSceneName));

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                float buttonWidth = 80;
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth * 2 - 20);

                if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                    showNewSceneDialog = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
                    if (strlen(newSceneName) > 0) {
                        createNewScene(newSceneName);
                        showNewSceneDialog = false;
                        memset(newSceneName, 0, sizeof(newSceneName));
                    }
                }
            }
            ImGui::End();
        }

        if (showSaveSceneAsDialog) {
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(350, 130), ImGuiCond_Appearing);

            if (ImGui::Begin("Save Scene As", &showSaveSceneAsDialog,
                            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking)) {
                ImGui::Text("Scene Name:");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##SaveSceneAsName", saveSceneAsName, sizeof(saveSceneAsName));

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                float buttonWidth = 80;
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth * 2 - 20);

                if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                    showSaveSceneAsDialog = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Save", ImVec2(buttonWidth, 0))) {
                    if (strlen(saveSceneAsName) > 0) {
                        projectManager.currentProject.currentSceneName = saveSceneAsName;
                        saveCurrentScene();
                        showSaveSceneAsDialog = false;
                        memset(saveSceneAsName, 0, sizeof(saveSceneAsName));
                    }
                }
            }
            ImGui::End();
        }
        
        // OBJ Import dialog
        if (showImportOBJDialog) {
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(400, 160), ImGuiCond_Appearing);

            if (ImGui::Begin("Import OBJ Model", &showImportOBJDialog,
                            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking)) {
                ImGui::Text("File: %s", fs::path(pendingOBJPath).filename().string().c_str());
                ImGui::TextDisabled("%s", pendingOBJPath.c_str());
                
                ImGui::Spacing();
                
                ImGui::Text("Object Name:");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##ImportOBJName", importOBJName, sizeof(importOBJName));

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                float buttonWidth = 80;
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth * 2 - 20);

                if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                    showImportOBJDialog = false;
                    pendingOBJPath.clear();
                }
                ImGui::SameLine();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.4f, 1.0f));
                if (ImGui::Button("Import", ImVec2(buttonWidth, 0))) {
                    importOBJToScene(pendingOBJPath, importOBJName);
                    showImportOBJDialog = false;
                    pendingOBJPath.clear();
                    memset(importOBJName, 0, sizeof(importOBJName));
                }
                ImGui::PopStyleColor(2);
            }
            ImGui::End();
        }
    }

    void renderProjectBrowserPanel() {
        ImGui::Begin("Project", &showProjectBrowser);

        if (!projectManager.currentProject.isLoaded) {
            ImGui::TextDisabled("No project loaded");
            ImGui::End();
            return;
        }

        ImGui::TextColored(ImVec4(0.4f, 0.7f, 0.95f, 1.0f), "[P] %s", projectManager.currentProject.name.c_str());
        if (projectManager.currentProject.hasUnsavedChanges) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "*");
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Scenes", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("+ New Scene")) {
                showNewSceneDialog = true;
                memset(newSceneName, 0, sizeof(newSceneName));
            }

            ImGui::Spacing();

            auto scenes = projectManager.currentProject.getSceneList();
            for (const auto& scene : scenes) {
                bool isCurrentScene = (scene == projectManager.currentProject.currentSceneName);

                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf |
                                          ImGuiTreeNodeFlags_SpanAvailWidth |
                                          ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (isCurrentScene) flags |= ImGuiTreeNodeFlags_Selected;

                ImGui::TreeNodeEx(scene.c_str(), flags, "[S] %s", scene.c_str());

                if (ImGui::IsItemClicked() && !isCurrentScene) {
                    loadScene(scene);
                }

                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Load") && !isCurrentScene) {
                        loadScene(scene);
                    }
                    if (ImGui::MenuItem("Duplicate")) {
                        addConsoleMessage("Scene duplication not yet implemented.", ConsoleMessageType::Info);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete") && !isCurrentScene) {
                        fs::remove(projectManager.currentProject.getSceneFilePath(scene));
                        addConsoleMessage("Deleted scene: " + scene, ConsoleMessageType::Info);
                    }
                    ImGui::EndPopup();
                }
            }

            if (scenes.empty()) {
                ImGui::TextDisabled("No scenes yet");
            }
        }
        
        if (ImGui::CollapsingHeader("Loaded Meshes")) {
            const auto& meshes = g_objLoader.getAllMeshes();
            if (meshes.empty()) {
                ImGui::TextDisabled("No meshes loaded");
                ImGui::TextDisabled("Import .obj files from File Browser");
            } else {
                for (size_t i = 0; i < meshes.size(); i++) {
                    const auto& mesh = meshes[i];
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf |
                                              ImGuiTreeNodeFlags_SpanAvailWidth |
                                              ImGuiTreeNodeFlags_NoTreePushOnOpen;
                    
                    ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "[M] %s", mesh.name.c_str());
                    
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("Vertices: %d", mesh.vertexCount);
                        ImGui::Text("Faces: %d", mesh.faceCount);
                        ImGui::Text("Has Normals: %s", mesh.hasNormals ? "Yes" : "No");
                        ImGui::Text("Has UVs: %s", mesh.hasTexCoords ? "Yes" : "No");
                        ImGui::TextDisabled("%s", mesh.path.c_str());
                        ImGui::EndTooltip();
                    }
                    
                    // Right-click to add instance to scene
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Add to Scene")) {
                            int id = nextObjectId++;
                            SceneObject obj(mesh.name, ObjectType::OBJMesh, id);
                            obj.meshPath = mesh.path;
                            obj.meshId = static_cast<int>(i);
                            sceneObjects.push_back(obj);
                            selectedObjectId = id;
                            projectManager.currentProject.hasUnsavedChanges = true;
                            addConsoleMessage("Added mesh instance: " + mesh.name, ConsoleMessageType::Info);
                        }
                        ImGui::EndPopup();
                    }
                }
            }
        }

        if (ImGui::CollapsingHeader("Folders", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGuiTreeNodeFlags folderFlags = ImGuiTreeNodeFlags_Leaf |
                                            ImGuiTreeNodeFlags_SpanAvailWidth |
                                            ImGuiTreeNodeFlags_NoTreePushOnOpen;

            ImGui::TreeNodeEx("Assets", folderFlags, "[D] Assets");
            if (ImGui::IsItemClicked()) {
                fileBrowser.currentPath = projectManager.currentProject.assetsPath;
                fileBrowser.needsRefresh = true;
            }

            ImGui::TreeNodeEx("Scripts", folderFlags, "[D] Scripts");
            if (ImGui::IsItemClicked()) {
                fileBrowser.currentPath = projectManager.currentProject.scriptsPath;
                fileBrowser.needsRefresh = true;
            }

            ImGui::TreeNodeEx("Scenes", folderFlags, "[D] Scenes");
            if (ImGui::IsItemClicked()) {
                fileBrowser.currentPath = projectManager.currentProject.scenesPath;
                fileBrowser.needsRefresh = true;
            }
        }

        ImGui::End();
    }

    void addConsoleMessage(const std::string& message, ConsoleMessageType type) {
        std::string prefix;
        switch (type) {
            case ConsoleMessageType::Info: prefix = "[INFO]"; break;
            case ConsoleMessageType::Warning: prefix = "[WARN]"; break;
            case ConsoleMessageType::Error: prefix = "[ERROR]"; break;
            case ConsoleMessageType::Success: prefix = "[SUCCESS]"; break;
        }

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::string timeStr = std::ctime(&time);
        timeStr = timeStr.substr(11, 8);

        consoleLog.push_back("[" + timeStr + "] " + prefix + " " + message);
        if (consoleLog.size() > 1000) {
            consoleLog.erase(consoleLog.begin());
        }
    }

    void logToConsole(const std::string& message) {
        addConsoleMessage(message, ConsoleMessageType::Info);
    }

    void applyModernTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.14f, 0.98f);

        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.24f, 0.28f, 1.00f);

        colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.36f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.26f, 0.30f, 1.00f);

        colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);

        colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);

        colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.08f, 0.10f, 0.75f);

        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.24f, 0.28f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.34f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.36f, 0.36f, 0.40f, 1.00f);

        colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.30f, 0.55f, 0.75f, 1.00f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.55f, 0.75f, 1.00f);

        colors[ImGuiCol_ResizeGrip] = ImVec4(0.24f, 0.24f, 0.28f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.30f, 0.55f, 0.75f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.30f, 0.55f, 0.75f, 0.95f);

        colors[ImGuiCol_DockingPreview] = ImVec4(0.30f, 0.55f, 0.75f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);

        colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.52f, 1.00f);

        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);

        colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.70f, 0.95f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.40f, 0.70f, 0.95f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.80f, 1.00f, 1.00f);

        style.WindowRounding = 4.0f;
        style.FrameRounding = 3.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 3.0f;
        style.TabRounding = 4.0f;
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.PopupBorderSize = 1.0f;
        style.WindowPadding = ImVec2(10, 10);
        style.FramePadding = ImVec2(6, 4);
        style.ItemSpacing = ImVec2(8, 6);
        style.ItemInnerSpacing = ImVec2(6, 4);
        style.IndentSpacing = 20.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 12.0f;
    }

    void setupDockspace() {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        if (firstFrame) {
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
            ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
            ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);

            ImGui::DockBuilderDockWindow("Hierarchy", dock_left);
            ImGui::DockBuilderDockWindow("Project", dock_left);
            ImGui::DockBuilderDockWindow("File Browser", dock_left);
            ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
            ImGui::DockBuilderDockWindow("Inspector", dock_right);
            ImGui::DockBuilderDockWindow("Console", dock_bottom);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::End();
    }

    void renderMainMenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                    showNewSceneDialog = true;
                    memset(newSceneName, 0, sizeof(newSceneName));
                }
                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                    saveCurrentScene();
                }
                if (ImGui::MenuItem("Save Scene As...")) {
                    showSaveSceneAsDialog = true;
                    strncpy(saveSceneAsName, projectManager.currentProject.currentSceneName.c_str(),
                           sizeof(saveSceneAsName) - 1);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Close Project")) {
                    if (projectManager.currentProject.hasUnsavedChanges) {
                        saveCurrentScene();
                    }
                    projectManager.currentProject = Project();
                    sceneObjects.clear();
                    selectedObjectId = -1;
                    showLauncher = true;
                    addConsoleMessage("Closed project", ConsoleMessageType::Info);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    glfwSetWindowShouldClose(editorWindow, true);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Duplicate", "Ctrl+D") && selectedObjectId != -1) {
                    duplicateSelected();
                }
                if (ImGui::MenuItem("Delete", "Delete") && selectedObjectId != -1) {
                    deleteSelected();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("GameObject")) {
                if (ImGui::BeginMenu("3D Object")) {
                    if (ImGui::MenuItem("Cube")) addObject(ObjectType::Cube, "Cube");
                    if (ImGui::MenuItem("Sphere")) addObject(ObjectType::Sphere, "Sphere");
                    if (ImGui::MenuItem("Capsule")) addObject(ObjectType::Capsule, "Capsule");
                    ImGui::Separator();
                    // Import OBJ from menu
                    if (ImGui::MenuItem("Import OBJ...")) {
                        addConsoleMessage("Use File Browser to import .obj files (double-click or right-click)", ConsoleMessageType::Info);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window")) {
                ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
                ImGui::MenuItem("Inspector", nullptr, &showInspector);
                ImGui::MenuItem("File Browser", nullptr, &showFileBrowser);
                ImGui::MenuItem("Project", nullptr, &showProjectBrowser);
                ImGui::MenuItem("Console", nullptr, &showConsole);
                ImGui::Separator();
                if (ImGui::MenuItem("Fullscreen Viewport", "F11", &viewportFullscreen)) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    addConsoleMessage("Modularity - V1.0.1", ConsoleMessageType::Info);
                }
                ImGui::EndMenu();
            }

            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "FPS: %.1f", 1.0f / deltaTime);

            ImGui::EndMainMenuBar();
        }
    }

    void renderHierarchyPanel() {
        ImGui::Begin("Hierarchy", &showHierarchy);

        static char searchBuffer[128] = "";
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##search", "Search objects...", searchBuffer, sizeof(searchBuffer));

        ImGui::Separator();

        if (ImGui::Button("+ Cube")) addObject(ObjectType::Cube, "Cube");
        ImGui::SameLine();
        if (ImGui::Button("+ Sphere")) addObject(ObjectType::Sphere, "Sphere");
        ImGui::SameLine();
        if (ImGui::Button("+ Capsule")) addObject(ObjectType::Capsule, "Capsule");

        ImGui::Separator();

        ImGui::BeginChild("SceneTree", ImVec2(0, 0), false);

        std::string filter = searchBuffer;
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

        for (size_t i = 0; i < sceneObjects.size(); i++) {
            if (sceneObjects[i].parentId != -1) continue;

            renderObjectNode(sceneObjects[i], filter);
        }

        ImGui::EndChild();

        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::BeginMenu("Create")) {
                if (ImGui::MenuItem("Cube")) addObject(ObjectType::Cube, "Cube");
                if (ImGui::MenuItem("Sphere")) addObject(ObjectType::Sphere, "Sphere");
                if (ImGui::MenuItem("Capsule")) addObject(ObjectType::Capsule, "Capsule");
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void renderObjectNode(SceneObject& obj, const std::string& filter) {
        std::string nameLower = obj.name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

        if (!filter.empty() && nameLower.find(filter) == std::string::npos) {
            return;
        }

        bool hasChildren = !obj.childIds.empty();
        bool isSelected = (selectedObjectId == obj.id);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;
        if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;

        const char* icon = "";
        switch (obj.type) {
            case ObjectType::Cube: icon = "[#]"; break;
            case ObjectType::Sphere: icon = "(O)"; break;
            case ObjectType::Capsule: icon = "[|]"; break;
            case ObjectType::OBJMesh: icon = "[M]"; break;  // OBJ mesh icon
        }

        bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)obj.id, flags, "%s %s", icon, obj.name.c_str());

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            selectedObjectId = obj.id;
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ImGui::SetDragDropPayload("SCENE_OBJECT", &obj.id, sizeof(int));
            ImGui::Text("Moving: %s", obj.name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_OBJECT")) {
                int draggedId = *(const int*)payload->Data;
                if (draggedId != obj.id) {
                    setParent(draggedId, obj.id);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Duplicate")) {
                selectedObjectId = obj.id;
                duplicateSelected();
            }
            if (ImGui::MenuItem("Delete")) {
                selectedObjectId = obj.id;
                deleteSelected();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Clear Parent") && obj.parentId != -1) {
                setParent(obj.id, -1);
            }
            ImGui::EndPopup();
        }

        if (nodeOpen) {
            for (int childId : obj.childIds) {
                auto it = std::find_if(sceneObjects.begin(), sceneObjects.end(),
                    [childId](const SceneObject& o) { return o.id == childId; });
                if (it != sceneObjects.end()) {
                    renderObjectNode(*it, filter);
                }
            }
            ImGui::TreePop();
        }
    }

    void renderFileBrowserPanel() {
        ImGui::Begin("File Browser", &showFileBrowser);

        if (fileBrowser.needsRefresh) {
            fileBrowser.refresh();
        }

        if (ImGui::Button("<")) {
            fileBrowser.navigateUp();
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) {
            fileBrowser.needsRefresh = true;
        }
        ImGui::SameLine();

        std::string pathStr = fileBrowser.currentPath.string();
        ImGui::TextWrapped("%s", pathStr.c_str());

        ImGui::Separator();

        ImGui::BeginChild("FileList", ImVec2(0, 0), false);

        for (const auto& entry : fileBrowser.entries) {
            const char* icon = fileBrowser.getFileIcon(entry);
            std::string filename = entry.path().filename().string();

            bool isSelected = (fileBrowser.selectedFile == entry.path());
            bool isOBJ = fileBrowser.isOBJFile(entry);

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

            // Highlight OBJ files
            if (isOBJ) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
            }

            ImGui::TreeNodeEx(filename.c_str(), flags, "%s %s", icon, filename.c_str());
            
            if (isOBJ) {
                ImGui::PopStyleColor();
            }

            if (ImGui::IsItemClicked()) {
                fileBrowser.selectedFile = entry.path();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (entry.is_directory()) {
                    fileBrowser.navigateTo(entry.path());
                } else if (isOBJ) {
                    // Double-click OBJ to import
                    pendingOBJPath = entry.path().string();
                    std::string defaultName = entry.path().stem().string();
                    strncpy(importOBJName, defaultName.c_str(), sizeof(importOBJName) - 1);
                    showImportOBJDialog = true;
                } else {
                    logToConsole("Selected file: " + filename);
                }
            }

            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Open")) {
                    if (entry.is_directory()) {
                        fileBrowser.navigateTo(entry.path());
                    }
                }
                // Add Import option for OBJ files
                if (isOBJ) {
                    if (ImGui::MenuItem("Import to Scene")) {
                        pendingOBJPath = entry.path().string();
                        std::string defaultName = entry.path().stem().string();
                        strncpy(importOBJName, defaultName.c_str(), sizeof(importOBJName) - 1);
                        showImportOBJDialog = true;
                    }
                    if (ImGui::MenuItem("Quick Import")) {
                        importOBJToScene(entry.path().string(), "");
                    }
                }
                if (ImGui::MenuItem("Show in Explorer")) {
                    #ifdef _WIN32
                    std::string cmd = "explorer \"" + entry.path().parent_path().string() + "\"";
                    system(cmd.c_str());
                    #endif
                }
                ImGui::EndPopup();
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void renderInspectorPanel() {
        ImGui::Begin("Inspector", &showInspector);

        if (selectedObjectId == -1) {
            ImGui::TextDisabled("No object selected");
            ImGui::End();
            return;
        }

        auto it = std::find_if(sceneObjects.begin(), sceneObjects.end(),
            [this](const SceneObject& obj) { return obj.id == selectedObjectId; });

        if (it == sceneObjects.end()) {
            ImGui::TextDisabled("Object not found");
            ImGui::End();
            return;
        }

        SceneObject& obj = *it;

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));

        if (ImGui::CollapsingHeader("Object Info", ImGuiTreeNodeFlags_DefaultOpen)) {
            char nameBuffer[128];
            strncpy(nameBuffer, obj.name.c_str(), sizeof(nameBuffer));
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';

            ImGui::Text("Name:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer))) {
                obj.name = nameBuffer;
                projectManager.currentProject.hasUnsavedChanges = true;
            }

            ImGui::Text("Type:");
            ImGui::SameLine();
            const char* typeNames[] = { "Cube", "Sphere", "Capsule", "OBJ Mesh" };
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "%s", typeNames[(int)obj.type]);

            ImGui::Text("ID:");
            ImGui::SameLine();
            ImGui::TextDisabled("%d", obj.id);
        }

        ImGui::PopStyleColor();

        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.4f, 0.5f, 0.3f, 1.0f));

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent(10.0f);

            ImGui::Text("Position");
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat3("##Position", &obj.position.x, 0.1f)) {
                projectManager.currentProject.hasUnsavedChanges = true;
            }
            ImGui::PopItemWidth();

            ImGui::Spacing();

            ImGui::Text("Rotation");
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat3("##Rotation", &obj.rotation.x, 1.0f, -360.0f, 360.0f)) {
                projectManager.currentProject.hasUnsavedChanges = true;
            }
            ImGui::PopItemWidth();

            ImGui::Spacing();

            ImGui::Text("Scale");
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat3("##Scale", &obj.scale.x, 0.05f, 0.01f, 100.0f)) {
                projectManager.currentProject.hasUnsavedChanges = true;
            }
            ImGui::PopItemWidth();

            ImGui::Spacing();

            if (ImGui::Button("Reset Transform", ImVec2(-1, 0))) {
                obj.position = glm::vec3(0.0f);
                obj.rotation = glm::vec3(0.0f);
                obj.scale = glm::vec3(1.0f);
                projectManager.currentProject.hasUnsavedChanges = true;
            }

            ImGui::Unindent(10.0f);
        }

        ImGui::PopStyleColor();

        // OBJ Mesh info section
        if (obj.type == ObjectType::OBJMesh) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.5f, 0.4f, 1.0f));
            
            if (ImGui::CollapsingHeader("Mesh Info", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent(10.0f);
                
                const auto* meshInfo = g_objLoader.getMeshInfo(obj.meshId);
                if (meshInfo) {
                    ImGui::Text("Source File:");
                    ImGui::TextDisabled("%s", fs::path(meshInfo->path).filename().string().c_str());
                    
                    ImGui::Spacing();
                    
                    ImGui::Text("Vertices: %d", meshInfo->vertexCount);
                    ImGui::Text("Faces: %d", meshInfo->faceCount);
                    ImGui::Text("Has Normals: %s", meshInfo->hasNormals ? "Yes" : "No");
                    ImGui::Text("Has UVs: %s", meshInfo->hasTexCoords ? "Yes" : "No");
                    
                    ImGui::Spacing();
                    
                    if (ImGui::Button("Reload Mesh", ImVec2(-1, 0))) {
                        std::string errMsg;
                        // Force reload by clearing and reloading
                        int newId = g_objLoader.loadOBJ(obj.meshPath, errMsg);
                        if (newId >= 0) {
                            obj.meshId = newId;
                            addConsoleMessage("Reloaded mesh: " + obj.name, ConsoleMessageType::Success);
                        } else {
                            addConsoleMessage("Failed to reload: " + errMsg, ConsoleMessageType::Error);
                        }
                    }
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Mesh data not found!");
                    ImGui::TextDisabled("Path: %s", obj.meshPath.c_str());
                    
                    if (ImGui::Button("Try Reload", ImVec2(-1, 0))) {
                        std::string errMsg;
                        obj.meshId = g_objLoader.loadOBJ(obj.meshPath, errMsg);
                        if (obj.meshId >= 0) {
                            addConsoleMessage("Mesh reloaded successfully", ConsoleMessageType::Success);
                        } else {
                            addConsoleMessage("Reload failed: " + errMsg, ConsoleMessageType::Error);
                        }
                    }
                }
                
                ImGui::Unindent(10.0f);
            }
            
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();

        if (renderer.getSkybox()) {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.5f, 0.3f, 0.5f, 1.0f));

            if (ImGui::CollapsingHeader("Environment")) {
                ImGui::Indent(10.0f);

                float timeOfDay = renderer.getSkybox()->getTimeOfDay();

                ImGui::Text("Time of Day");
                ImGui::PushItemWidth(-1);
                if (ImGui::SliderFloat("##TimeOfDay", &timeOfDay, 0.0f, 1.0f, "%.2f")) {
                    renderer.getSkybox()->setTimeOfDay(timeOfDay);
                }
                ImGui::PopItemWidth();

                if (ImGui::Button("Night", ImVec2(60, 0))) renderer.getSkybox()->setTimeOfDay(0.0f);
                ImGui::SameLine();
                if (ImGui::Button("Sunrise", ImVec2(60, 0))) renderer.getSkybox()->setTimeOfDay(0.25f);
                ImGui::SameLine();
                if (ImGui::Button("Day", ImVec2(60, 0))) renderer.getSkybox()->setTimeOfDay(0.5f);
                ImGui::SameLine();
                if (ImGui::Button("Sunset", ImVec2(60, 0))) renderer.getSkybox()->setTimeOfDay(0.75f);

                ImGui::Unindent(10.0f);
            }

            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Duplicate", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 4, 0))) {
            duplicateSelected();
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Delete", ImVec2(-1, 0))) {
            deleteSelected();
        }
        ImGui::PopStyleColor(2);

        ImGui::End();
    }

    void renderConsolePanel() {
        ImGui::Begin("Console", &showConsole);

        if (ImGui::Button("Clear")) {
            consoleLog.clear();
        }
        ImGui::SameLine();
        static bool autoScroll = true;
        ImGui::Checkbox("Auto-scroll", &autoScroll);

        ImGui::Separator();

        ImGui::BeginChild("LogArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        for (const auto& log : consoleLog) {
            ImVec4 color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
            if (log.find("error") != std::string::npos || log.find("Error") != std::string::npos || log.find("[ERROR]") != std::string::npos) {
                color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            } else if (log.find("warning") != std::string::npos || log.find("Warning") != std::string::npos || log.find("[WARN]") != std::string::npos) {
                color = ImVec4(1.0f, 0.8f, 0.4f, 1.0f);
            } else if (log.find("success") != std::string::npos || log.find("Success") != std::string::npos || log.find("[SUCCESS]") != std::string::npos) {
                color = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
            }
            ImGui::TextColored(color, "%s", log.c_str());
        }

        if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void renderViewport() {
        ImGuiWindowFlags viewportFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

        if (viewportFullscreen) {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            viewportFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport", nullptr, viewportFlags);
        ImGui::PopStyleVar();

        ImVec2 vsize = ImGui::GetContentRegionAvail();
        if (vsize.x > 0 && vsize.y > 0) {
            viewportWidth = static_cast<int>(vsize.x);
            viewportHeight = static_cast<int>(vsize.y);
            if (rendererInitialized) {
                renderer.resize(viewportWidth, viewportHeight);
            }
        }

        if (rendererInitialized) {
            renderer.beginRender(camera.getViewMatrix(), glm::perspective(glm::radians(FOV), (float)viewportWidth / viewportHeight, NEAR_PLANE, FAR_PLANE));
            renderer.renderScene(camera, sceneObjects);
            unsigned int tex = renderer.getViewportTexture();
            ImGui::Image((void*)(intptr_t)tex, vsize, ImVec2(0, 1), ImVec2(1, 0));
        }

        bool mouseOverImage = ImGui::IsItemHovered();
        bool windowFocused = ImGui::IsWindowFocused();

        viewportController.updateFocusFromImGui(windowFocused);

        if (mouseOverImage && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            viewportController.setFocused(true);
            viewportController.clearManualUnfocus();
            cursorLocked = true;
            glfwSetInputMode(editorWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            camera.firstMouse = true;
        }

        ImGui::SetCursorPos(ImVec2(10, 30));
        ImGui::TextColored(ImVec4(1, 1, 1, 0.7f), "WASD: Move | QE: Up/Down | Shift: Sprint | ESC: Release | F11: Fullscreen");

        if (viewportController.isViewportFocused()) {
            ImGui::SetCursorPos(ImVec2(10, 50));
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Camera Active");
        }

        ImGui::End();
    }

    void addObject(ObjectType type, const std::string& baseName) {
        int id = nextObjectId++;
        std::string name = baseName + " " + std::to_string(id);
        sceneObjects.push_back(SceneObject(name, type, id));
        selectedObjectId = id;
        if (projectManager.currentProject.isLoaded) {
            projectManager.currentProject.hasUnsavedChanges = true;
        }
        logToConsole("Created: " + name);
    }

    void duplicateSelected() {
        auto it = std::find_if(sceneObjects.begin(), sceneObjects.end(),
            [this](const SceneObject& obj) { return obj.id == selectedObjectId; });

        if (it != sceneObjects.end()) {
            int id = nextObjectId++;
            SceneObject newObj(it->name + " (Copy)", it->type, id);
            newObj.position = it->position + glm::vec3(1.0f, 0.0f, 0.0f);
            newObj.rotation = it->rotation;
            newObj.scale = it->scale;
            // Copy mesh data for OBJ meshes
            newObj.meshPath = it->meshPath;
            newObj.meshId = it->meshId;
            
            sceneObjects.push_back(newObj);
            selectedObjectId = id;
            if (projectManager.currentProject.isLoaded) {
                projectManager.currentProject.hasUnsavedChanges = true;
            }
            logToConsole("Duplicated: " + newObj.name);
        }
    }

    void deleteSelected() {
        auto it = std::remove_if(sceneObjects.begin(), sceneObjects.end(),
            [this](const SceneObject& obj) { return obj.id == selectedObjectId; });

        if (it != sceneObjects.end()) {
            logToConsole("Deleted object");
            sceneObjects.erase(it, sceneObjects.end());
            selectedObjectId = -1;
            if (projectManager.currentProject.isLoaded) {
                projectManager.currentProject.hasUnsavedChanges = true;
            }
        }
    }

    void setParent(int childId, int parentId) {
        auto childIt = std::find_if(sceneObjects.begin(), sceneObjects.end(),
            [childId](const SceneObject& obj) { return obj.id == childId; });

        if (childIt == sceneObjects.end()) return;

        if (childIt->parentId != -1) {
            auto oldParentIt = std::find_if(sceneObjects.begin(), sceneObjects.end(),
                [&childIt](const SceneObject& obj) { return obj.id == childIt->parentId; });
            if (oldParentIt != sceneObjects.end()) {
                auto& children = oldParentIt->childIds;
                children.erase(std::remove(children.begin(), children.end(), childId), children.end());
            }
        }

        childIt->parentId = parentId;

        if (parentId != -1) {
            auto newParentIt = std::find_if(sceneObjects.begin(), sceneObjects.end(),
                [parentId](const SceneObject& obj) { return obj.id == parentId; });
            if (newParentIt != sceneObjects.end()) {
                newParentIt->childIds.push_back(childId);
            }
        }

        if (projectManager.currentProject.isLoaded) {
            projectManager.currentProject.hasUnsavedChanges = true;
        }
        logToConsole("Reparented object");
    }

    void setupImGui() {
        float mainScale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        applyModernTheme();

        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(mainScale);
        style.FontScaleDpi = mainScale;

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

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
