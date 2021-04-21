#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aTexCoord;

out vec3 TexCoord;
out vec4 ourColor;

uniform mat4 model_mat;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 custom_color;


void main()
{
    gl_Position = projection * view * model_mat * vec4(aPos, 1.0);
    ourColor = custom_color;
    // ourColor = glm::vec4(0.7, 0.7, 0.7, 1.0);
    TexCoord = aTexCoord;
}

