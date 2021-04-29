#version 330 core
out vec4 FragColor ;

uniform sampler2D scene;
in vec2 vTexCoords;

void main()
{
    FragColor =  texture(scene, vTexCoords);
}