#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 WorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;    
    mat4 transform = projection * view * model;
    gl_Position = transform * vec4(aPos, 1.0);
    mat3 normalMat = transpose(inverse(mat3(model)));
    Normal = normalize(normalMat * aNormal); 
    WorldPos = vec3(model * vec4(aPos, 1.0));
}