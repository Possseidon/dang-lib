#version 460 core

// #pragma optimize(off)
// #pragma debug(on)

#include "quaternion.glsl"

uniform mat4 projection_matrix;
uniform mat2x4 modelview_transform;

uniform mat4 mvp;

in vec3 v_pos;
in vec2 v_texcoord;
in vec3 v_offset;
in vec3 v_color;

out vec2 f_texcoord;
out vec3 f_color;

void main()
{
    f_texcoord = v_pos.xy + 1e-6 * v_texcoord * length(vec2(1, 0));
    f_color = v_color;
    vec3 pos = applyTransform(modelview_transform, v_pos + v_offset);
    gl_Position = projection_matrix * vec4(pos, 1);
}
