#version 430 core

layout (location = 0) in vec3 pos;	
layout (location = 1) in vec2 texCoord;		
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec3 vsPos;
out vec3 vsNormal;
out vec2 vsTexCoord;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vsPos = vec3(model * vec4(pos, 1.0));
    vsNormal = mat3(transpose(inverse(model))) * normal;    
	mat3 NormalMat = mat3(transpose(inverse(model)));
	vsTexCoord = texCoord;
	vec3 T = normalize(NormalMat * tangent);
	vec3 B = normalize(NormalMat * bitangent);
	vec3 N = normalize(vsNormal);
	mat3 TBN = transpose(mat3(T, B, N));

    gl_Position = projection * view * vec4(vsPos, 1.0);
}