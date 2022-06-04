vec4 quat_mul(vec4 a, vec4 b)
{
    return vec4((a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y),
                (a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x),
                (a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w),
                (a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z));
}

mat2x4 dquat(vec4 q) { return mat2x4(q, vec4(0)); }

mat2x4 dquat(vec3 v) { return mat2x4(vec4(0, 0, 0, 1), vec4(v, 0)); }

mat2x4 dquat_conjugate(mat2x4 q) { return mat2x4(-q[0].xyz, q[0].w, q[1].xyz, -q[1].w); }

mat2x4 dquat_mul(mat2x4 a, mat2x4 b)
{
    return mat2x4(quat_mul(b[0], a[0]), quat_mul(b[1], a[0]) + quat_mul(b[0], a[1]));
}

vec3 applyTransform(mat2x4 q, vec3 v) { return dquat_mul(dquat_mul(dquat_conjugate(q), dquat(v)), q)[1].xyz; }
