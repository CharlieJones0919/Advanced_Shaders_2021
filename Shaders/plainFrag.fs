#version 410 core
out vec4 FragColor ;

uniform sampler2D texture_diffuse1;
in vec3 vNormals;
in vec2 vTexCoords;

void main()
{
    FragColor = texture(texture_diffuse1, vTexCoords);
}