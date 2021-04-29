#version 450 core
out vec4 FragColor;

in vec3 gPos;
in vec3 gNormals;
in vec2 gTexCoords;
in float gVisibility;
in vec3 gEyePos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
};                                                                        

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}; 

uniform float heightMapScale;
uniform sampler2D snowTex;
uniform sampler2D rockTex;
uniform sampler2D grassTex;
uniform sampler2D waterTex;

uniform vec3 sky;

uniform DirLight dirLight;
uniform Material mat;

void main()
{   
    vec3 viewDir = normalize(gEyePos - gPos);
	vec3 norm = normalize(gNormals) ;
	vec3 ambient = dirLight.ambient * mat.ambient;     
    vec3 lightDir = normalize(-dirLight.direction);
	 
    // diffuse shading
    float diff = max(dot(norm, dirLight.direction), 0.0);
    
	// specular shading
    vec3 reflectDir = reflect(-dirLight.direction, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    
	// combine results
    vec3 diffuse  = dirLight.diffuse  * (diff * mat.diffuse);
    vec3 specular = dirLight.specular * (spec * mat.specular);
  
	float fragHeight = (gPos.y - gNormals.y) / heightMapScale;
	vec3 colour;   
	
	if (fragHeight < 0.175f)                      
	{          
		colour = vec3(texture(waterTex, gTexCoords));
	} 
	else if (fragHeight >= 0.175f)  
    {      
		colour = vec3(mix(vec3(texture(grassTex, gTexCoords)), vec3(texture(rockTex, gTexCoords)), smoothstep(0.10,0.35, fragHeight)).rgb);
	}  
	else if (fragHeight >= 0.35f)
	{     
		colour = vec3(mix(vec3(texture(rockTex, gTexCoords)), vec3(texture(snowTex, gTexCoords )), smoothstep(0.5,0.75, fragHeight)).rgb);
		colour = vec3(mix(vec3(texture(grassTex, gTexCoords)), colour, smoothstep(0.15,0.10, fragHeight)));
	} 
	else if (fragHeight >= 0.45f)  
	{  
		colour = vec3(texture(snowTex, gTexCoords));
	} 		
	
	FragColor = vec4(vec3(ambient + diffuse + specular) * colour, 1.0f);
	FragColor = mix(vec4(sky, 1.0), FragColor, gVisibility);
}