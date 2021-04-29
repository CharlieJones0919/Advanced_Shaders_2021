#version 410 core
out vec4 FragColor ;

uniform sampler2D grassTex;
in vec3 vNormals;
in vec2 vTexCoords;

void main()
{
    FragColor = texture(grassTex, vTexCoords);
}