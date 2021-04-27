#version 450 core
layout(triangles) in ;
layout(triangle_strip, max_vertices = 3) out ;
vec3 getNormal() ;

in vec3 tEPos[];
in vec2 tETexCoords[];

out vec3 gPos;
out vec3 gNormals;
out vec2 gTexCoords;

void main()
{
   for(int i = 0 ; i < 3; i++)
   {
      gl_Position = gl_in[i].gl_Position;
      gPos = tEPos[i] ;
      gNormals = getNormal() ;    
	  gTexCoords = tETexCoords[i];
      EmitVertex() ;
   }
     EndPrimitive() ;
}


vec3 getNormal()
{
    vec3 a = vec3(gl_in[1].gl_Position) - vec3(gl_in[0].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[0].gl_Position);
    return normalize(cross(a, b));
}