#version 460 core
#extension GL_ARB_shading_language_include : require
#include "/include/light.glsl"

#define MAX_STEPS 128
#define MAX_LIGHT_STEPS 16

#define absorptionCoef 1
#define scatteringCoef 4
#define extinctionCoef (absorptionCoef + scatteringCoef)

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 cameraPos;
uniform vec3 AABBMin;
uniform vec3 AABBMax;

uniform uint gridSize;
uniform sampler3D SDF;

in vec3 WorldPos;
out vec4 FragColor;

bool AABBIntersect(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax, out vec3 near, out vec3 far);
vec3 worldPos2Coord(vec3 worldPos);
float getTransmittanceLight(vec3 pos, vec3 lightDir, vec3 lightPos, bool directional);
vec3 calculateLightVolume(vec3 pos, vec3 viewDir);

void main()
{
    // camera ==> pos
    vec3 dir = normalize(WorldPos - cameraPos);
    vec3 near, far;
    if(!AABBIntersect(WorldPos, dir, AABBMin, AABBMax, near, far))
    {
        discard;
    }

    vec4 nearScreenPos = projection * view * vec4(near, 1.0);
    nearScreenPos /= nearScreenPos.w;

    // discard back face if near face is in the camera
    if(length(WorldPos - far) < 1e-3 && nearScreenPos.z < 1.0 && nearScreenPos.z > -1.0)
    {
        discard;
    }

    // camera is inside the box
    if((cameraPos - near).x / dir.x > 0)
    {
        near = cameraPos;
    }

    float stepSize = length(far - near) / float(MAX_STEPS);
    vec3 color = vec3(0.0);
    float transmittance = 1.0;

    // ray marching loop
    for(int i = 0; i < MAX_STEPS; i++)
    {
        vec3 pos = near + dir * stepSize * float(i + 0.5);
        vec3 coord = worldPos2Coord(pos);
        float density = texture(SDF, coord).r;
        transmittance *= exp(-density * extinctionCoef * stepSize);

        vec3 inScattering = calculateLightVolume(pos, -dir);
        float outScattering = scatteringCoef * density;
        vec3 currentLight = inScattering * outScattering;

        color += transmittance * currentLight * stepSize;
    }
	FragColor = vec4(color / (1 - transmittance), 1 - transmittance);
}

bool AABBIntersect(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax, out vec3 near, out vec3 far)
{
    mat4 invModel = inverse(model);
    rayOrigin = (invModel * vec4(rayOrigin, 1.0)).xyz;
    rayDir = (invModel * vec4(rayDir, 0.0)).xyz;
    vec3 invDir = 1.0 / rayDir;
    vec3 t0s = (boxMin - rayOrigin) * invDir;
    vec3 t1s = (boxMax - rayOrigin) * invDir;
    vec3 tsmaller = min(t0s, t1s);
    vec3 tbigger = max(t0s, t1s);
    float tmin = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    float tmax = min(min(tbigger.x, tbigger.y), tbigger.z);
    near = (model * vec4(rayOrigin + rayDir * tmin, 1.0)).xyz;
    far = (model * vec4(rayOrigin + rayDir * tmax, 1.0)).xyz;
    return tmax > tmin;
}

vec3 worldPos2Coord(vec3 worldPos)
{
    vec3 localPos = (inverse(model) * vec4(worldPos, 1.0)).xyz;
    return (localPos - AABBMin) / (AABBMax - AABBMin);
}

float getTransmittanceLight(vec3 pos, vec3 lightDir, vec3 lightPos, bool directional = false)
{
    float stepSize = length((model * vec4(AABBMax, 1.0) - model * vec4(AABBMin, 1.0)).xyz) / MAX_LIGHT_STEPS / 2;
    if(!directional)
    {
        stepSize = min(stepSize, length(lightPos - pos) / MAX_LIGHT_STEPS);
    }
    float transmittance = 1;
    for(int i = 0; i < MAX_LIGHT_STEPS; i++)
    {
        vec3 coord = worldPos2Coord(pos);
        float density = texture(SDF, coord).r;
        transmittance *= exp(-density * extinctionCoef * stepSize);
        pos += lightDir * stepSize;
    }
    return 2 * transmittance * (1 - transmittance * transmittance);
}

// viewDir: pos ==> camera
vec3 calculateLightVolume(vec3 pos, vec3 viewDir)
{
    vec3 baseColor = vec3(0.3);
    vec3 result = baseColor + GL_AmbientLight;

    // calculate directional light   
    vec3 lightDir = -vec3(GL_DirectionalLight.direction);
    float phase = dualLobPhase(0.5, -0.5, 0.3, dot(lightDir, viewDir));
    float transmittance = getTransmittanceLight(pos, lightDir, vec3(0), true);
    float intensity = GL_DirectionalLight.intensity;
    result += phase * transmittance * intensity * vec3(GL_DirectionalLight.color);

    // calculate point light
    for(int i = 0; i < GL_Num_PointLight; i++)
    {
        float distance = length(vec3(GL_PointLight[i].position) - pos);
        lightDir = normalize(vec3(GL_PointLight[i].position) - pos);
        phase = dualLobPhase(0.5, -0.5, 0.3, dot(lightDir, viewDir));
        transmittance = getTransmittanceLight(pos, lightDir, vec3(GL_PointLight[i].position), false);
        intensity = GL_PointLight[i].intensity / (distance * distance);
        result += phase * transmittance * intensity * vec3(GL_PointLight[i].color);
    }
    
    // calculate spot light
    for(int i = 0; i < GL_Num_SpotLight; i++)
    {
        float distance = length(vec3(GL_SpotLight[i].position) - pos);
        lightDir = normalize(vec3(GL_SpotLight[i].position) - pos);
        phase = dualLobPhase(0.5, -0.5, 0.3, dot(lightDir, viewDir));
        transmittance = getTransmittanceLight(pos, lightDir, vec3(GL_SpotLight[i].position), false);
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
        result += phase * transmittance * intensity * vec3(GL_SpotLight[i].color);
    }    
    return result;
}



