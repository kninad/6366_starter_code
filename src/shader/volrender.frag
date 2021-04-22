#version 330 core

in vec4 ourColor;
in vec3 TexCoord;

out vec4 FragColor;

uniform sampler3D ourTexture;

void main() {    
    // FragColor = ourColor;
    // float a = texture(ourTexture, TexCoord).rrrr;
    FragColor = texture(ourTexture, TexCoord).rrrr * ourColor;
}