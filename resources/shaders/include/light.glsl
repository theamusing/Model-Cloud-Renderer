#ifndef LIGHT_GLSL
#define LIGHT_GLSL

struct PointLight {
    vec4 position;
    vec4 color;
    float intensity;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct SpotLight {
    vec4 position;
    vec4 direction;
    vec4 color;
    float intensity;
    float cutOff;
    float outerCutOff;
};

struct Material {
    float kd;
    float ks;
};

uniform uint GL_Num_PointLight;
uniform uint GL_Num_SpotLight;

uniform vec3 GL_AmbientLight;
uniform DirectionalLight GL_DirectionalLight;

layout(std430, binding=0) buffer GL_PointLight_Buffer
{
    PointLight GL_PointLight[];
};

layout(std430, binding=1) buffer GL_SpotLight_Buffer
{
    SpotLight GL_SpotLight[];
};

// viewDir: pos ==> camera
vec3 calculateLight(vec3 pos, vec3 normal, vec3 viewDir, vec3 kd, vec3 ks)
{
    // here is a simple blinn-phong model
    vec3 result = kd * GL_AmbientLight;
    if(dot(viewDir, normal) <= 0)
        return result;

    // calculate directional light
    vec3 lightDir = -vec3(GL_DirectionalLight.direction);
    vec3 halfDir = normalize(lightDir + viewDir);
    float intensity = GL_DirectionalLight.intensity;
    result += (kd * max(dot(lightDir, normal) + 0.1, 0) + ks * pow(max(dot(halfDir, normal), 0), 8)) * intensity * vec3(GL_DirectionalLight.color);


    // calculate point light
    for(int i = 0; i < GL_Num_PointLight; i++)
    {
        float distance = length(vec3(GL_PointLight[i].position) - pos);
        lightDir = normalize(vec3(GL_PointLight[i].position) - pos);
        halfDir = normalize(lightDir + viewDir);
        intensity = GL_PointLight[i].intensity / (distance * distance);
        result += (kd * max(dot(lightDir, normal) + 0.1, 0) + ks * pow(max(dot(halfDir, normal), 0), 8)) * intensity * vec3(GL_PointLight[i].color);
    }
    
    // calculate spot light
    for(int i = 0; i < GL_Num_SpotLight; i++)
    {
        float distance = length(vec3(GL_SpotLight[i].position) - pos);
        lightDir = normalize(vec3(GL_SpotLight[i].position) - pos);
        halfDir = normalize(lightDir + viewDir);
        float spotEffect = dot(-vec3(GL_SpotLight[i].direction), lightDir);
        if(spotEffect > cos(GL_SpotLight[i].cutOff))
        {
            spotEffect = 1.0;
        }
        else if(spotEffect <= cos(GL_SpotLight[i].cutOff) && spotEffect > cos(GL_SpotLight[i].outerCutOff))
        {
            spotEffect = (spotEffect - cos(GL_SpotLight[i].outerCutOff)) / (cos(GL_SpotLight[i].cutOff) - cos(GL_SpotLight[i].outerCutOff));
        }
        else
        {
            spotEffect = 0;
        }
        intensity = spotEffect * GL_SpotLight[i].intensity / (distance * distance);
        result += (kd * max(dot(lightDir, normal) + 0.1, 0) + ks * pow(max(dot(halfDir, normal), 0), 8)) * intensity * vec3(GL_SpotLight[i].color);
    }    

    return result;
}


#endif /* LIGHT_GLSL */