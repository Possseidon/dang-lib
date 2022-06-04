#version 460 core

// #pragma optimize(off)
// #pragma debug(on)

#include "quaternion.glsl"

uniform mat4 projection_matrix;
uniform mat2x4 modelview_transform;

in vec2 v_texcoord;
in vec3 v_pos;
in float v_radius;
in vec3 v_color;

out vec2 f_texcoord;
out float f_radius;
out vec3 f_color;
out vec4 f_pos;

void main()
{
    f_texcoord = v_texcoord;
    f_radius = v_radius;
    f_color = v_color;
    gl_Position = vec4(applyTransform(modelview_transform, v_pos), 1);
    gl_Position.xy += v_texcoord * v_radius * vec2(1, 1);
    f_pos = gl_Position;
    gl_Position = projection_matrix * gl_Position;
}
