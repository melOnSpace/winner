#version 330 core
out vec4 frag_color;

in vec2 uv_cords;
uniform sampler2D screenshot_texture;
// uniform vec3 flashlight;
// uniform int fl_enabled;
// uniform float aratio;

void main() {
    frag_color = texture(screenshot_texture, uv_cords);
    frag_color = vec4(frag_color.z, frag_color.y, frag_color.x, frag_color.w);
}
// if (fl_enabled != 0) {
//     vec2 pixel_pos = (uv_cords * 2.0) - 1.0;
//     pixel_pos.x *= aratio;
//     vec2 dist = flashlight.xy - pixel_pos;
//     if (sqrt(dist.x * dist.x + dist.y * dist.y) > flashlight.z) {
//         frag_color *= vec4(0.25, 0.25, 0.25, 1.0);
//     } else {
//         frag_color *= vec4(1.0, 1.0, 1.0, 1.0);
//     }
// }
