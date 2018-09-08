#pragma once

#include "vector_math.h"
#include "utils.h"

struct OrientedBoundingBox
{
    vec2 origin;
    vec2 size;
    f32 angle;
};

struct CircleBound
{
    vec2 center;
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

    inline void setPos(vec2 pos) {
        switch(type) {
            case OBB: obb.origin = pos; break;
            case CIRCLE: cb.center = pos; break;
        }
    }

    inline vec2 getPos() const {
        switch(type) {
            case OBB: return obb.origin;
            case CIRCLE: return cb.center;
        }
        assert(0);
        return vec2{};
    }
};

template<typename Payload>
struct ColliderWithPayload
{
    Collider collider;
    Payload payload;
};

struct CollisionInfo
{
    vec2 penVec;
};

struct PhysBody
{
    vec2 prevPos;
    vec2 pos;
    vec2 vel;
    f32 weight;
    f32 bounceStrength;
};

struct PhysWorld
{
    Array<Collider> colStatic;
    ArraySparse<Collider> colDynamic;
    ArraySparse<PhysBody> bodyDyn;

    PhysWorld();
    void clear();
    Collider* addStaticCollider(Collider col);
    PhysBody* addDynamicBody(Collider col, PhysBody body, i32* bid = nullptr);
    void removeBodyById(i32 bid);

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

void physWorldInit();
PhysWorld& getPhysWorld();
