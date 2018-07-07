#pragma once

#include "base.h"
#include <bx/math.h>

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
};

inline vec3 operator+(const vec3& v1, const vec3& v2) {
    vec3 v = v1;
    v.x += v2.x;
    v.y += v2.y;
    v.z += v2.z;
    return v;
}

inline vec3 operator-(const vec3& v1, const vec3& v2) {
    vec3 v = v1;
    v.x -= v2.x;
    v.y -= v2.y;
    v.z -= v2.z;
    return v;
}

inline vec3 operator*(vec3 v, f32 scalar) {
    v.x *= scalar;
    v.y *= scalar;
    v.z *= scalar;
    return v;
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
    vec2 v = v1;
    v.x += v2.x;
    v.y += v2.y;
    return v;
}

inline vec2 operator-(const vec2& v1, const vec2& v2) {
    vec2 v = v1;
    v.x -= v2.x;
    v.y -= v2.y;
    return v;
}

inline vec2 operator*(vec2 v, f32 scalar) {
    v.x *= scalar;
    v.y *= scalar;
    return v;
}

inline f32 vec2Dot(vec2 v1, vec2 v2)
{
    return v1.x * v2 .x + v1.y * v2.y;
}

union quat
{
    struct { f32 x, y, z, w; };
    f32 data[4];
    operator f32*() { return data; }
    operator const f32*() const { return data; }
};

struct mat4
{
    f32 data[16];
    operator f32*() { return data; }
    operator const f32*() const { return data; }
};


inline quat vec3RotDiffQuat(vec3 v1, vec3 v2)
{
    vec3 c;
    bx::vec3Cross(c, v2, v1);
    f32 v1Len = bx::vec3Length(v1);
    f32 v2Len = bx::vec3Length(v2);

    quat q;
    q.x = c.x;
    q.y = c.y;
    q.z = c.z;
    q.w = bx::sqrt((v1Len * v1Len) * (v2Len * v2Len)) + bx::vec3Dot(v1, v2);
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

