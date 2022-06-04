#version 460 core

uniform float time;
uniform int texture_index;
uniform sampler2DArray diffuse_map;
uniform mat2 uv_bounds = mat2(1);

in vec3 f_pos;
in vec3 f_color;
in vec3 f_normal;
flat in float f_maxlod;

out vec4 outcolor;

vec4 textureLimitLod(sampler2DArray tex, vec3 texcoord)
{
    vec2 pos = mix(uv_bounds[0], uv_bounds[1], texcoord.xy);
    float lod = min(textureQueryLod(tex, pos).x, f_maxlod);
    ivec3 tex_size = textureSize(tex, 0);
    vec2 fixed_inset = pow(2, lod - 0.75) / vec2(tex_size.xy);
    vec2 inset = mix(vec2(0), fixed_inset, min(lod, 1));
    vec2 fract_pos = mix(uv_bounds[0] + inset, uv_bounds[1] - inset, fract(texcoord.xy));
    return textureLod(tex, vec3(fract_pos, texcoord.z), lod);
}

void main()
{
    vec3 normal = normalize(f_normal);
    vec3 normal_abs = abs(normal);
    float normal_sum = normal_abs.x + normal_abs.y + normal_abs.z;
    normal_abs /= normal_sum;
    vec3 light_dir = vec3(sin(time) * 2, 5, cos(time) * 2);
    float light = dot(normal, normalize(light_dir)) / 2 + 0.5;
    vec3 factor = f_pos / 2;
    vec3 diffuse = mat3(textureLimitLod(diffuse_map, vec3(factor.yz, texture_index)).rgb,
                        textureLimitLod(diffuse_map, vec3(factor.zx, texture_index)).rgb,
                        textureLimitLod(diffuse_map, vec3(factor.xy, texture_index)).rgb) *
                   normal_abs;
    // vec3 diffuse = texture(diffuse_map, vec3(mix(uv_bounds[0], uv_bounds[1], factor.xy), texture_index)).rgb;
    // if (any(equal(step(mod(f_pos - 0.025, 1.0), vec3(0.95)), vec3(0.0))))
    //   diffuse *= 0.5;
    outcolor = vec4(light * f_color * diffuse, 1.0);
    if (ivec3(outcolor.rgb * 255 + 0.5) == ivec3(255, 0, 255))
        outcolor.rgb = vec3(255, 1, 255) / 255;
}
