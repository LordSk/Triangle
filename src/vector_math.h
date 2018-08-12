#pragma once

#include "base.h"
#include <bx/math.h>
#include <math.h>

inline f32 signf(f32 f)
{
    return f < 0.0f ? -1.0f : 1.0f;
}

union vec4
{
    struct { f32 x, y, z, w; };
    f32 data[4];
    operator f32*() { return data; }
    operator const f32*() const { return data; }

    inline vec4& operator +=(const vec4& v2) {
        x += v2.x;
        y += v2.y;
        z += v2.z;
        w += v2.w;
        return *this;
    }

    inline vec4& operator -=(const vec4& v2) {
        x -= v2.x;
        y -= v2.y;
        z -= v2.z;
        w -= v2.w;
        return *this;
    }
};

inline vec4 vec4Splat(f32 one)
{
    return vec4{one, one, one, one};
}

union vec3
{
    struct { f32 x, y, z; };
    f32 data[3];
    operator f32*() { return data; }
    operator const f32*() const { return data; }

    inline vec3& operator +=(const vec3& v2) {
        x += v2.x;
        y += v2.y;
        z += v2.z;
        return *this;
    }

    inline vec3& operator -=(const vec3& v2) {
        x -= v2.x;
        y -= v2.y;
        z -= v2.z;
        return *this;
    }

    inline vec3 operator-() {
        vec3 v;
        v.x = -x;
        v.y = -y;
        v.z = -z;
        return v;
    }
};

inline vec3 vec3Splat(f32 one)
{
    return vec3{one, one, one};
}

inline vec3 operator+(const vec3& v1, const vec3& v2) {
    return vec3{v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

inline vec3 operator-(const vec3& v1, const vec3& v2) {
    return vec3{v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

inline vec3 operator*(vec3 v, f32 scalar) {
    return vec3{v.x * scalar, v.y * scalar, v.z * scalar};
}

inline vec3 operator*(const vec3& v1, const vec3& v2) {
    return vec3{v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

inline f32 vec3Len(vec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline vec3 vec3Norm(vec3 v1)
{
    return v1 * (1.0f / vec3Len(v1));
}

union vec2
{
    struct { f32 x, y; };
    f32 data[2];
    operator f32*() { return data; }
    operator const f32*() const { return data; }

    inline vec2& operator +=(const vec2& v2) {
        x += v2.x;
        y += v2.y;
        return *this;
    }

    inline vec2& operator -=(const vec2& v2) {
        x -= v2.x;
        y -= v2.y;
        return *this;
    }
};

inline vec2 operator+(const vec2& v1, const vec2& v2) {
    return vec2{ v1.x + v2.x, v1.y + v2.y };
}

inline vec2 operator-(const vec2& v1, const vec2& v2) {
    return vec2{ v1.x - v2.x, v1.y - v2.y };
}

inline vec2 operator*(const vec2& v1, f32 scalar) {
    vec2 v = v1;
    v.x *= scalar;
    v.y *= scalar;
    return v;
}

inline f32 vec2Dot(vec2 v1, vec2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

inline f32 vec2Len(vec2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

inline vec2 vec2Norm(vec2 v1)
{
    return v1 * (1.0f / vec2Len(v1));
}

inline vec3 vec2ToVec3(vec2 v1)
{
    return vec3{ v1.x, v1.y, 0 };
}

inline vec2 vec3ToVec2(vec3 v1)
{
    return vec2{ v1.x, v1.y };
}

inline vec2 vec2Rotate(const vec2& v1, f32 angle)
{
    const f32 c = cosf(angle);
    // FIXME: quaternion rotation is left-handed(?) while world is RH and vice-versa
    const f32 s = -sinf(angle);
    const f32 tx = v1.x;
    const f32 ty = v1.y;
    vec2 v;
    v.x = (c * tx) - (s * ty);
    v.y = (s * tx) + (c * ty);
    return v;
}

inline vec2 vec2Rotate(const vec2& v1, f32 cos_a, f32 sin_a)
{
    const f32 c = cos_a;
    // FIXME: quaternion rotation is left-handed(?) while world is RH and vice-versa
    const f32 s = -sin_a;
    const f32 tx = v1.x;
    const f32 ty = v1.y;
    vec2 v;
    v.x = (c * tx) - (s * ty);
    v.y = (s * tx) + (c * ty);
    return v;
}

inline vec2 vec2Lerp(const vec2& v1, const vec2& v2, f32 l)
{
    return v1 + (v2 - v1) * l;
}

union quat
{
    struct { f32 x, y, z, w; };
    f32 data[4];
    operator f32*() { return data; }
    operator const f32*() const { return data; }
};

#define quat_identity quat{ 0, 0, 0, 1 }

struct mat4
{
    f32 data[16];
    operator f32*() { return data; }
    operator const f32*() const { return data; }
};


inline quat vec3RotDiffQuat(vec3 v1, vec3 v2)
{
    bx::vec3Norm(v1, v1);
    bx::vec3Norm(v2, v2);
    const f32 d = bx::vec3Dot(v1, v2);

    if(d > 0.999999f) {
        return quat_identity;
    }
    if(d < -0.99999f) {
        quat q;
        vec3 right;
        vec3 up = {0, 0, 1};
        bx::vec3Cross(right, v1, up);
        if(bx::vec3Length(right) < 0.0001f) {
            vec3 xv = {1, 0, 0};
            bx::vec3Cross(right, v1, xv);
        }
        assert(bx::vec3Length(right) > 0.0001f);
        bx::quatRotateAxis(q, right, bx::kPi);
        return q;
    }

    vec3 c;
    bx::vec3Cross(c, v2, v1);
    f32 s = bx::sqrt((1.0f + d) * 2.0f);
    f32 invs = 1.0f / s;

    quat q;
    q.x = c.x * invs;
    q.y = c.y * invs;
    q.z = c.z * invs;
    q.w = s * 0.5f;
    bx::quatNorm(q, q);
    return q;
}


// ldir and pn must be normalized
inline bool planeLineIntersection(vec3* out, vec3 l0, vec3 ldir, vec3 p0, vec3 pn)
{
    f32 d = bx::vec3Dot(ldir, pn);
    if(d == 0) {
        return false;
    }

    vec3 d0 = p0 - l0;
    f32 si = bx::vec3Dot(pn, d0) / d;
    *out = l0 + ldir * si;
    return true;
}

struct Transform
{
    vec3 pos = {0, 0, 0};
    vec3 scale = {1, 1, 1};
    quat rot = {0, 0, 0, 1};

    inline void toMtx(mat4* mtxModel) const {
        mat4 mtxTrans, mtxRot, mtxScale;

        bx::mtxTranslate(mtxTrans, pos.x, pos.y, pos.z);
        bx::mtxQuat(mtxRot, rot);
        bx::mtxScale(mtxScale, scale.x, scale.y, scale.z);

        // scale * rot * trans
        mat4 mtx1;
        bx::mtxMul(mtx1, mtxScale, mtxRot);
        bx::mtxMul(*mtxModel, mtx1, mtxTrans);
    }
};
