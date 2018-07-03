#pragma once
#include "base.h"
#include <SDL2/SDL_events.h>

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

inline vec3 operator+(const vec3& v1, const vec3& v2) {
    vec3 v = v1;
    v.x += v2.x;
    v.y += v2.y;
    v.z += v2.z;
    return v;
}

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
    vec3 pos;
    vec3 scale;
    quat rotation;
    mat4 mtxModel;

    vec3 vel;

    struct {
        bool8 left;
        bool8 right;
        bool8 up;
        bool8 down;
    } input = {};

    PlayerShip();
    void computeModelMatrix();
    void handleEvent(const SDL_Event& event);
    void update(f64 delta);
};
