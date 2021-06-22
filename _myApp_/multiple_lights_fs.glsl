#version 430 core

in vec3 vsNormal;  
in vec3 vsPos;  
in vec2 vsTexCoord;
in mat3 TBN;


struct Material { 
    sampler2D diffuse; 
    sampler2D specular; 
    float shininess; 

	vec3 defaultAmbient;
	vec3 defaultDiffuse;
	vec3 defaultSpecular;
	int useDiffuseMap;
	int useSpecularMap;
}; 

struct DirLight { 
    vec3 direction; 

    vec3 ambient; 
    vec3 diffuse; 
    vec3 specular; 
}; 

struct PointLight {
    vec3 position;
    
    float c1;
    float c2;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float c1;
    float c2;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NUM_POINT_LIGHTS 2 

uniform Material material;
uniform vec3 viewPos; 
uniform DirLight dirLight;
uniform PointLight pointLights[NUM_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform int useNormal;

uniform sampler2D normalMap;
uniform int useNormalMap;


out vec4 fragColor;

// 광원별 컬러 계산 함수 정의
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 matAmbientColor;
vec3 matDiffuseColor;
vec3 matSpecularColor;


void main()
{
	if(useNormal != 0)
	{
		// properties
		vec3 norm = normalize(vsNormal);
		vec3 vsPos1 = vsPos;
		vec3 viewDir = normalize(viewPos - vsPos1);
		matAmbientColor = material.defaultAmbient;
		matDiffuseColor = material.defaultDiffuse;
		matSpecularColor = material.defaultSpecular;
		if(useNormalMap != 0)
		{
		    norm = vec3(texture(normalMap, vsTexCoord));
		    norm = normalize(norm *2.0 - 1.0);
		    vsPos1 = TBN * vsPos;
		    viewDir = normalize(TBN*(viewPos - vsPos));

			
		}


		if(material.useDiffuseMap != 0)
		{
			matAmbientColor = vec3(texture(material.diffuse, vsTexCoord));
			matDiffuseColor = vec3(texture(material.diffuse, vsTexCoord));
		}

		if(material.useSpecularMap != 0)
		{
			matSpecularColor = vec3(texture(material.specular, vsTexCoord));
		}
    
		// phase 1: directional lighting
		vec3 result = CalcDirLight(dirLight, norm, viewDir);
		// phase 2: point lights
		for(int i = 0; i < NUM_POINT_LIGHTS; i++) 
		{
			result += CalcPointLight(pointLights[i], norm, vsPos1, viewDir);    
		}
		// phase 3: spot light
		result += CalcSpotLight(spotLight, norm, vsPos1, viewDir);    
    
		fragColor = vec4(result, 1.0);
	}

	else
		fragColor = vec4(material.defaultDiffuse, 1.0);

} 

// directional light
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    if(useNormalMap !=0)
	{
	   vec3 lightDir = normalize(TBN* -light.direction);
	}
	
	
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * matAmbientColor;
    vec3 diffuse = light.diffuse * diff * matDiffuseColor;
    vec3 specular = light.specular * spec * matSpecularColor;
    
	return (ambient + diffuse + specular);
}

// point light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    if(useNormalMap !=0)
	{
	   vec3 lightDir = normalize(TBN * light.position - fragPos);
	}
	
	
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (1.0 + light.c1 * distance + light.c2 * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * matAmbientColor;
    vec3 diffuse = light.diffuse * diff * matDiffuseColor;
    vec3 specular = light.specular * spec * matSpecularColor;

    return (ambient + diffuse + specular) * attenuation;
}

// spot light
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	
	vec3 lightDir = normalize(light.position - fragPos);
	if(useNormalMap !=0)
	{
	   vec3 lightDir = normalize(TBN*light.position - fragPos);
	}


	
    
	// diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
	// specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
	// Soft edge spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (1.0 + light.c1 * distance + light.c2 * (distance * distance));
        
	// combine results
    vec3 ambient = light.ambient * matAmbientColor;
    vec3 diffuse = light.diffuse * diff * matDiffuseColor;
    vec3 specular = light.specular * spec * matSpecularColor;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}