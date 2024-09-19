#version 460 core
#extension GL_ARB_shading_language_include : require
#include "/include/light.glsl"

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 WorldPos;

uniform sampler2D texture_diffuse1;
// uniform sampler2D texture_specular1;

uniform vec3 cameraPos;

void main()
{    
    // cauculate lighting
    vec3 norm = normalize(Normal);
    // directional light
    vec3 kd = vec3(texture(texture_diffuse1, TexCoords));
    vec3 ks = vec3(0.2);
    // vec3 ks = vec3(texture(texture_specular1, TexCoords));

    FragColor = vec4(calculateLight(WorldPos, norm, normalize(cameraPos-WorldPos), kd, ks), 1);
}

