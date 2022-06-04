#version 460 core

uniform mat4 projection_matrix;

in vec2 f_texcoord;
in float f_radius;
in vec3 f_color;
in vec4 f_pos;

out vec4 outcolor;
layout(depth_less) out float gl_FragDepth;

void main()
{
    const vec2 border = vec2(1.0 - 1.0 / 1.0, 0.0);

    // 1    -> 0.0   // 0.0  border (circle)
    // 0.75 -> 0.25  // 0.25 border
    // 0.5  -> 0.5   // 0.5  border
    // 0.25 -> 0.75  // 0.75 border
    // 0    -> 1.0   // 1.0  border (square)

    vec2 abs_texcoord = abs(f_texcoord);
    vec2 offset_texcoord = max(abs_texcoord - border, 0) / (1 - border);
    float len = length(offset_texcoord);
    if (len > 1)
        discard;
    float height = sqrt(1 - len * len);
    outcolor = vec4((height / 2 + 0.5) * f_color, 1);

    vec4 pos = f_pos;
    pos.z += height * f_radius;
    pos = projection_matrix * pos;
    pos /= pos.w;
    gl_FragDepth = pos.z / 2 + 0.5;
}
