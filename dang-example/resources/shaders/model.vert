#version 460 core

// #pragma optimize(off)
// #pragma debug(on)

#include "quaternion.glsl"

uniform mat4 projection_matrix;
uniform mat2x4 modelview_transform;
uniform float time;

uniform mat4 mvp;

in vec3 v_pos;
in vec3 v_color;
in vec3 v_normal;
in float v_maxlod;

out vec3 f_pos;
out vec3 f_color;
out vec3 f_normal;
flat out float f_maxlod;

float rand(vec2 co)
{
    return 0;
    // return cos(sin(dot(co, vec2(12.9898, 78.233))) * (43758.5453 + time * 0.5)) * 0.2;
}

void main()
{
    vec3 pos = v_pos + vec3(rand(v_pos.xy), rand(v_pos.yz), rand(v_pos.zx));
    f_pos = pos;
    f_color = v_color;
    f_normal = v_normal;
    f_maxlod = v_maxlod;
    pos = applyTransform(modelview_transform, pos);
    gl_Position = projection_matrix * vec4(pos, 1);
}
