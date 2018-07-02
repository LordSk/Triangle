#pragma once
#include "mesh_load.h"

// TODO: move this
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

inline vec3 operator*(vec3 v, f32 scalar) {
    v.x *= scalar;
    v.y *= scalar;
    v.z *= scalar;
    return v;
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
};

struct PlayerShip
{
    MeshHandle meshHnd;
    vec3 pos;
    vec3 scale;
    quat rotation;
    mat4 mtxModel;

    void setMesh(MeshHandle hnd_) {
        meshHnd = hnd_;
    }

    void computeModelMatrix();
};
