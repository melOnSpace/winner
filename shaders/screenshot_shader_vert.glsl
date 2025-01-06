#version 330 core
layout (location = 0) in vec4 position;

out vec2 uv_cords;
uniform vec3 transforms;

void main() {
    gl_Position = vec4(position.xy * transforms.zz + transforms.xy, 0.0, 1.0);
    uv_cords = vec2(position.z, 1.0 - position.w);
}
