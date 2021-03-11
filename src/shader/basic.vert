#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture;

out vec4 ourColor;

uniform vec4 custom_color;

uniform mat4 model_mat;
uniform mat4 view;
uniform mat4 projection;


void main(){
    gl_Position = projection * view * model_mat * vec4(position, 1.0);
    
    ourColor = custom_color;
}