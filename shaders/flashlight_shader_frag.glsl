#version 330 core
out vec4 frag_color;

in vec2 uv_cords;
uniform vec3 flashlight;
uniform float aratio;
uniform int flash_enabled;

void main() {
    frag_color = vec4(0.0, 0.0, 0.0, 0.0);
    if (flash_enabled == 0) return;

    vec2 pixel_pos = (uv_cords * 2.0) - 1.0;
    pixel_pos.x *= aratio;
    vec2 dist = flashlight.xy - pixel_pos;
    if (sqrt(dist.x * dist.x + dist.y * dist.y) > flashlight.z)
        frag_color.w = 0.75;
}
