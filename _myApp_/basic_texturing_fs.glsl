#version 430 core

in vec3 vsColor;
in vec2 vsTexCoord;
uniform sampler2D texture1;

out vec4 fragColor;
				
void main(void)	
{				
	vec4 c1 = texture(texture1, vsTexCoord);
	fragColor = c1;
}					