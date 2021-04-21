#version 330 core

in vec4 ourColor;
in vec3 TexCoord;

out vec4 FragColor;

uniform sampler3D ourTexture;

void main() {    
    FragColor = ourColor;
    // FragColor = texture(ourTexture, TexCoord);
}