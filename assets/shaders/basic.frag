#version 460

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = vertexColor;
}