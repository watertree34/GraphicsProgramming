#version 430 core

in vec3 vsNormal;  
in vec3 vsPos;  
in vec2 vsTexCoord;

struct Material { 
    sampler2D diffuse; 
    sampler2D specular; 
    float shininess; 
}; 

struct Light { 
    vec3 position; 

    vec3 ambient; 
    vec3 diffuse; 
    vec3 specular; 
}; 

uniform Material material;
uniform Light light; 
uniform vec3 viewPos; 

out vec4 fragColor;

void main()
{
    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, vsTexCoord));
  	
    // diffuse 
    vec3 norm = normalize(vsNormal);
    vec3 lightDir = normalize(light.position - vsPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, vsTexCoord));
    
    // specular
    vec3 viewDir = normalize(viewPos - vsPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.diffuse, vsTexCoord));  
        
    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0);
} 