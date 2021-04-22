#version 330 core

in vec4 ourColor;
in vec3 TexCoord;

out vec4 FragColor;

uniform sampler3D ourTexture;
uniform bool transferFuncSign;

void main() { 
    // FragColor = ourColor;
    // float a = texture(ourTexture, TexCoord).rrrr;
    if(transferFuncSign)
    {
        FragColor = texture(ourTexture, TexCoord).rrrr * ourColor;
    }
    else
    {
        FragColor = ourColor;
    }   
    
}