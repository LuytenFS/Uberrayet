#version 460

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 FragColor;

#ifdef VULKAN
layout(set = 0, binding = 0) uniform sampler2D uTexture;
#else
uniform sampler2D uTexture;
#endif

void main() {
    FragColor = texture(uTexture, texCoord) * vertexColor;
}