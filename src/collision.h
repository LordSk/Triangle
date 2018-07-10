#pragma once

#include "vector_math.h"
#include "utils.h"

struct OrientedBoundingBox
{
    vec3 origin;
    vec3 size;
    f32 angle;
};

struct CircleBound
{
    vec3 center;
    f32 radius;
};

struct Collider
{
    enum Enum: i32 {
        OBB = 0,
        CIRCLE
    };

    Enum type;

    union {
        OrientedBoundingBox obb;
        CircleBound cb;
    };

    inline Collider& makeObb(OrientedBoundingBox obb_) {
        type = OBB;
        obb = obb_;
        return *this;
    }

    inline Collider& makeCb(CircleBound cb_) {
        type = CIRCLE;
        cb = cb_;
        return *this;
    }

    inline void setPos(vec3 pos) {
        switch(type) {
            case OBB: obb.origin = pos; break;
            case CIRCLE: cb.center = pos; break;
        }
    }

    inline vec3 getPos() const {
        switch(type) {
            case OBB: return obb.origin;
            case CIRCLE: return cb.center;
        }
        assert(0);
        return vec3{};
    }
};

struct CollisionInfo
{
    vec2 penVec;
};

struct PhysBody
{
    Collider col;
    vec3 pos;
    vec3 vel;
    f32 weight;
    f32 bounceStrength;
};

struct PhysWorld
{
    Array<Collider> colStatic;
    Array<PhysBody> bodies;

    PhysWorld();
    void clear();
    Collider* addStaticCollider(Collider col);
    PhysBody* addDynamicBody(PhysBody body);

    void update(f64 delta, const i32 stepCount);
    void dbgDraw(vec3 offset);
};

bool obbIntersectObb(const OrientedBoundingBox& obbA, const OrientedBoundingBox& obbB, CollisionInfo* out);
bool obbIntersectCb(const OrientedBoundingBox& obbA, const CircleBound& cbB, CollisionInfo* out);
bool cbIntersectCb(const CircleBound& cbA, const CircleBound& cbB, CollisionInfo* out);
bool colliderIntersect(const Collider& col1, const Collider& col2, CollisionInfo* out);

void obbDbgDraw(const OrientedBoundingBox& obb, vec4 color);
void cbDbgDraw(const CircleBound& cb, vec4 color);
void colliderDbgDraw(const Collider& col1, vec4 color);
