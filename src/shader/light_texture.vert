#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out mat3 TBN;

uniform mat4 model_mat;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
    // vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    // vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));    
    // mat3 TBN = mat3(T, B, N);

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));    
    T = normalize(T - dot(T, N) * N); // re-orthogonalize T with respect to N    
    vec3 B = cross(N, T); // orthogonal vector B = cross product of T and N
    mat3 TBN = mat3(T, B, N);

    FragPos = vec3(model_mat * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model_mat))) * normal;
    TexCoord = vec2(texture.x, 1.0 - texture.y);
    gl_Position = projection * view * model_mat * vec4(position, 1.0);    
}