#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

// uniform vec3 object_color;
// uniform vec3 view_position;

void main()
{
    // FragColor = vec4(object_color, 1.0);
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}