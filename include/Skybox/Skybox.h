#ifndef SKYBOX_H
#define SKYBOX_H

class Shader;

class Skybox {
private:
    unsigned int VAO, VBO;
    Shader* skyboxShader;
    float timeOfDay = 0.5f; // 0.0 = night, 0.25 = sunrise, 0.5 = day, 0.75 = sunset, 1.0 = midnight

    void setupMesh();

public:
    Skybox();
    ~Skybox();
    
    void draw(const float* view, const float* projection);
    void setTimeOfDay(float time); // 0.0 to 1.0
    float getTimeOfDay() const { return timeOfDay; }
};

#endif
