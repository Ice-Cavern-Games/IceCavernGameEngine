#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
}
proj;

layout(set = 2, binding = 0) uniform ColorProperty {
    vec4 color;
}
materialColor;

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 view;
}
mv;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec2 fragTexCoord;

void main() {
    gl_Position = proj.proj * mv.view * mv.model * vec4(inPosition, 1.0);
    normal = mat3(transpose(inverse(mv.view * mv.model))) * inNormal;
    fragColor = materialColor.color;
    fragPos = vec3(mv.view * mv.model * vec4(inPosition, 1.0));
    fragTexCoord = inTexCoord;
}