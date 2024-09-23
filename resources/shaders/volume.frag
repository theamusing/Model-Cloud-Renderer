#version 460 core
#extension GL_ARB_shading_language_include : require
#include "/include/light.glsl"

#define MAX_STEPS 256
#define extinctionCoef 0.05

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

void main()
{
    vec3 baseColor = vec3(1);

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
        color += transmittance * density * baseColor * stepSize;
    }
	FragColor = vec4(color, transmittance);
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




