#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;			
layout (location = 2) in vec2 texCoord;		
layout (location = 3) in vec3 normal;

out vec3 vsPos;
out vec3 vsNormal;
out vec2 vsTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vsPos = vec3(model * vec4(pos, 1.0));
    vsNormal = mat3(transpose(inverse(model))) * normal;  
	vsTexCoord = texCoord;
    
    gl_Position = projection * view * vec4(vsPos, 1.0);
}