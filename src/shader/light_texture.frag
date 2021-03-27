#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TBN;

uniform sampler2D ourTexture;
uniform sampler2D normalMap;
uniform bool on_diffuseMap;
uniform bool on_normalMap;

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
    vec3 color  = texture(ourTexture, TexCoord).rgb;
    vec3 nm_normal = texture(normalMap, TexCoord).rgb;
    nm_normal = nm_normal * 2.0 - 1.0; // Use this instead of normHat if normalMap.
    nm_normal = normalize(nm_normal);
    //nm_normal = normalize(TBN * nm_normal);

    if (on_Dlight)
    {
        if(on_normalMap)
        {
            final += CalcDirLight(dlight_dir, dlight_amb, dlight_dif, dlight_spc, nm_normal, viewDir);
        }
        else
        {
            final += CalcDirLight(dlight_dir, dlight_amb, dlight_dif, dlight_spc, normHat, viewDir);
        }       
        
    }

    if(on_Plight)
    {   
        if(on_normalMap)
        {
            final += CalcPointLight(plight_pos, plight_amb, plight_dif, plight_spc, nm_normal, FragPos, viewDir);
        }
        else
        {
            final += CalcPointLight(plight_pos, plight_amb, plight_dif, plight_spc, normHat, FragPos, viewDir);
        }
    }
    
    if(on_diffuseMap)
    {
        final *= color;    
    }
    
    FragColor = vec4(final, 1.0);
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