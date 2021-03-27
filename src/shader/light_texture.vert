#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model_mat;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model_mat * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model_mat))) * normal;
    TexCoord = vec2(texture.x, 1.0 - texture.y);
    gl_Position = projection * view * model_mat * vec4(position, 1.0);    
}