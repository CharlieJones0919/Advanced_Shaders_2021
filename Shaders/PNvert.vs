#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;

uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;
//uniform vec3 eyePos ;

//out vec3 eye ;
out vec3 vPos; 
out vec3 vNormals ;

void main()
{
    normals = normalize((model * vec4(aNormals, 0.0)).xyz) ;
	//eye = eyePos ;
	vPos = (model * vec4(aPos, 1.0)).xyz; 
}