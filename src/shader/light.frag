#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 object_color;
uniform vec3 view_position;

uniform bool on_Dlight;
uniform bool on_Plight;

uniform vec3 dlight_dir;
uniform vec3 dlight_amb;
uniform vec3 dlight_dif;
uniform vec3 dlight_spc;

uniform vec3 plight_pos;
uniform vec3 plight_amb;
uniform vec3 plight_dif;
uniform vec3 plight_spc;

vec3 CalcDirLight (vec3 direction, vec3 ambient, vec3 diffuse, vec3 specular, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(vec3 position, vec3 ambient, vec3 diffuse, vec3 specular, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 normHat = normalize(Normal);
    vec3 viewDir = normalize(view_position - FragPos);
    vec3 final = vec3(0.0, 0.0, 0.0);
    if (on_Dlight)
    {
        final += CalcDirLight(dlight_dir, dlight_amb, dlight_dif, dlight_spc, normHat, viewDir);
    }
    if(on_Plight)
    {
        final += CalcPointLight(plight_pos, plight_amb, plight_dif, plight_spc, normHat, FragPos, viewDir);
    }   
    
    // final *= object_color;
    FragColor = vec4(final, 1.0);

    // FragColor = vec4(object_color, 1.0);
}

// Directional light.
vec3 CalcDirLight(vec3 direction, vec3 ambient, vec3 diffuse, vec3 specular, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 _ambient = ambient;
    vec3 _diffuse = diffuse * diff;
    vec3 _specular = specular * spec;
    return (_ambient + _diffuse + _specular);
}

// Point light.
vec3 CalcPointLight(vec3 position, vec3 ambient, vec3 diffuse, vec3 specular, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        
    vec3 _ambient = ambient;
    vec3 _diffuse = diffuse * diff;
    vec3 _specular = specular * spec;
    return (_ambient + _diffuse + _specular);
}