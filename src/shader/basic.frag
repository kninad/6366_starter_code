#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

struct Direction_Light {
    vec3 direction;
	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Point_Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 object_color;
uniform vec3 view_position;
uniform Direction_Light dirLight;
uniform Point_Light pntLight;

vec3 CalcDirLight(Direction_Light light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(Point_Light light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 normHat = normalize(Normal);
    vec3 viewDir = normalize(view_position - FragPos);

    vec3 final = CalcDirLight(dirLight, normHat, viewDir);
    final += CalcPointLight(pntLight, normHat, FragPos, viewDir);
    final *= object_color;
    FragColor = vec4(final, 1.0);
}

// Directional light.
vec3 CalcDirLight(Direction_Light light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    return (ambient + diffuse + specular);
}

// Point light.
vec3 CalcPointLight(Point_Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    return (ambient + diffuse + specular);
}