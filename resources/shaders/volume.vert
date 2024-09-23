#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 WorldPos;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
    WorldPos = vec3(model * vec4(aPos, 1.0));
}