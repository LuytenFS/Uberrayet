#version 460 core

in vec4 vertexColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;

void main() {
    // Fragment shader only calculates pixel output colors
    FragColor = texture(texture_diffuse1, TexCoord) * vertexColor;
}