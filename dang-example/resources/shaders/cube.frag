#version 460 core

uniform sampler2DArray diffuse_map;

uniform int texture_index;

in vec2 f_texcoord;
in vec3 f_color;

out vec4 outcolor;

void main()
{
    outcolor = texture(diffuse_map, vec3(f_texcoord, texture_index));
    outcolor.rgb += 0.5 * f_color;
    // outcolor.rgb *= 1e-6;
    // float level = textureQueryLod(diffuse_map, f_texcoord).x;
    // outcolor += vec4(level * 0.125, -level * 0.125, 0, 0);
}
