#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 model;

out vec3 vPos; 
out vec2 vTexCoords;

void main()
{
	vPos = (model * vec4(aPos, 1.0)).xyz; 
	vTexCoords = aTexCoords; 
}