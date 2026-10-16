#version 460

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) out vec4 vertexColor;
layout (location = 1) out vec2 TexCoord;

#ifdef VULKAN
layout(push_constant) uniform PushConstants { mat4 mvp; } pc;
#define MVP pc.mvp
#else
uniform mat4 u_mvp;
#define MVP u_mvp
#endif

void main() {
    gl_Position = MVP * vec4(aPos, 1.0);
    vertexColor = aColor;
    TexCoord = aTexCoord;
}