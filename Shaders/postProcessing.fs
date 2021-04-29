#version 330 core

float LinearizeDepth(float depth);
out vec4 FragColor;

uniform sampler2D scene;
in vec2 vTexCoords;

uniform int postProcessEffect = 0;

uniform bool isDepthBuffer;
uniform float nearPlane;
uniform float farPlane;

void main()
{
	if (!isDepthBuffer)
	{
		vec4 sceneTex = texture(scene, vTexCoords);
	
		if (postProcessEffect == 0) // None
		{
			FragColor = sceneTex;
		}
		else if (postProcessEffect == 1) // Negative
		{
			FragColor =  1.0 - sceneTex;
		}
		else if (postProcessEffect == 2) // Black & White
		{
			FragColor =  vec4(vec3((sceneTex.x + sceneTex.y + sceneTex.z) / 3), 1.0);
		}		
	}
	else
	{
		float depth = texture(scene, vTexCoords).r;
		FragColor = vec4(vec3(LinearizeDepth(depth) / farPlane), 1.0);
	}
}

float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}