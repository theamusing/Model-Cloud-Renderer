#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#define MAX_LIGHTS 10
#define PI 3.14159265359

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
};

struct alignas(16) PointLight {
    glm::vec4 position;
    glm::vec4 color;
    float intensity;
};

struct alignas(16) SpotLight {
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 color;
    float intensity;
    float cutOff;
    float outerCutOff;
};

class LightManager
{
public:
    LightManager()
    {
        // init ambient light
        ambientLight = glm::vec3(0.05f, 0.1f, 0.1f);

        // init directional light
        directionalLight.direction = glm::vec3(-1.0f, 0.0f, 0.0f);
        directionalLight.color = glm::vec3(1.0f, 0.4f, 0.0f);
        directionalLight.intensity = 0.4f;

        // init buffers
        glGenBuffers(1, &pointLightBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_LIGHTS * sizeof(PointLight), NULL, GL_STATIC_DRAW);

        glGenBuffers(1, &spotLightBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotLightBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_LIGHTS * sizeof(SpotLight), NULL, GL_STATIC_DRAW);
    }
    
    void clearLights()
    {
        pointLights.clear();
        spotLights.clear();
    }

    void setDirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity)
    {
        directionalLight.direction = normalize(direction);
        directionalLight.color = color;
        directionalLight.intensity = intensity;
    }

    void addPointLight(glm::vec3 position, glm::vec3 color, float intensity)
    {
        if(spotLights.size() >= MAX_LIGHTS)
        {
            return;
        }
        PointLight pointLight;
        pointLight.position = glm::vec4(position, 1.0);
        pointLight.color = glm::vec4(color, 1.0);
        pointLight.intensity = intensity;
        pointLights.push_back(pointLight);
        updateBuffers();
    }

    void removePointLight(int index)
    {
        if(index < pointLights.size())
        {
            pointLights.erase(pointLights.begin() + index);
            updateBuffers();
        }
    }

    void addSpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float intensity, float cutOff, float outerCutOff)
    {
        if(spotLights.size() >= MAX_LIGHTS)
        {
            return;
        }
        SpotLight spotLight;
        spotLight.position = glm::vec4(position, 1.0);
        spotLight.direction = glm::vec4(normalize(direction), 1.0);
        spotLight.color = glm::vec4(color, 1.0);
        spotLight.intensity = intensity;
        spotLight.cutOff = clamp(cutOff / 180.0f, 0.0f, 1.0f) * PI;
        spotLight.outerCutOff = clamp(outerCutOff / 180.0f , 0.0f, 1.0f) * PI;
        spotLights.push_back(spotLight);
        updateBuffers();
    }

    void removeSpotLight(int index)
    {
        if(index < spotLights.size())
        {
            spotLights.erase(spotLights.begin() + index);
            updateBuffers();
        }
    }

    void Apply(Shader &shader)
    {
        // set uniforms
        shader.setVec3("GL_AmbientLight", ambientLight);
        shader.setUint("GL_Num_PointLight", pointLights.size());
        shader.setUint("GL_Num_SpotLight", spotLights.size());
        shader.setVec3("GL_DirectionalLight.direction", directionalLight.direction);
        shader.setVec3("GL_DirectionalLight.color", directionalLight.color);
        shader.setFloat("GL_DirectionalLight.intensity", directionalLight.intensity);

        // bind buffers
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pointLightBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, spotLightBuffer);
    }
private:
    DirectionalLight directionalLight;
    glm::vec3 ambientLight;
    std::vector<PointLight> pointLights;
    std::vector<SpotLight> spotLights;

    GLuint pointLightBuffer, spotLightBuffer;

    void updateBuffers()
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, pointLights.size() * sizeof(PointLight), pointLights.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotLightBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, spotLights.size() * sizeof(SpotLight), spotLights.data());
    }
};

#endif
