#version 330 core
out vec4 FragColor;
  
in vec3 TexCoord;

uniform sampler3D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}