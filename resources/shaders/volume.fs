#version 460 core
#extension GL_ARB_shading_language_include : require
#include "/include/light.glsl"

#define MAX_STEPS 256
#define density 1 // use texture later
#define extinctionCoef 1 // use texture later

uniform mat4 model;
uniform vec3 cameraPos;
uniform vec3 AABBMin;
uniform vec3 AABBMax;

in vec3 WorldPos;
out vec4 FragColor;

bool AABBIntersect(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax, out vec3 near, out vec3 far);

void main()
{
    vec3 baseColor = vec3(1.0);

    vec3 dir = normalize(WorldPos - cameraPos);
    vec3 near, far;
    if(!AABBIntersect(WorldPos, dir, AABBMin, AABBMax, near, far))
    {
        discard;
    }
    // discard back face
    if((WorldPos - near).x / dir.x > 1e-3)
    {
        discard;
    }

    float stepSize = length(far - near) / float(MAX_STEPS);
    vec4 color = vec4(0.0);
    float transmittance = 1.0;

    // ray marching loop
    for(int i = 0; i < MAX_STEPS; i++)
    {
        vec3 pos = near + dir * stepSize * float(i + 0.5);

        transmittance *= exp(-density * extinctionCoef * stepSize);
        color += transmittance * vec4(baseColor, 1.0) * stepSize;
    }
	FragColor = color;
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




