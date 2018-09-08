#pragma once
#include "utils.h"
#include "collision.h"

struct DamageTeam
{
    enum Enum {
        NEUTRAL=0,
        PLAYER,
        ENEMY,
        _COUNT
    };
};

struct DamageFlag
{
    enum Enum: u32
    {
        SingleInstance = (1 << 0),
        DamageOverTime = (1 << 1)
    };
};

struct DamageEvent
{
    CollisionInfo collisionInfo;
    u32 fromEntityUID = 0;
    i32 fromEid = 0;
    u32 flags = 0; // DamageFlag
    f32 dmg = 0; // damage value
};

struct DamageCore
{
    u32 _entityUID; // dont fill
    i32 _eid; // dont fill
    u32 flags = 0; // DamageFlag
    f32 dmg = 0;
};

struct DamageFrame
{
    struct IntersectInfo {
        DamageTeam::Enum team1;
        DamageTeam::Enum team2;
        DamageCore pl1;
        DamageCore pl2;
        i32 pl1_cid;
        i32 pl2_cid;
        CollisionInfo collisionInfo;
    };


    // TODO: remove teams? accelerate search with a grid
    Array<Collider> colliders[DamageTeam::_COUNT];
    Array<DamageCore> payloads[DamageTeam::_COUNT];
    Array<IntersectInfo> intersectList;

    DamageFrame();

    // done post physical world update
    void registerCollider(const DamageTeam::Enum team, const Collider& c, const DamageCore& payload);
    void clearFrame();
    void resolveIntersections();
    void dbgDraw();
};

void dmgFrameInit();
DamageFrame& getDmgFrame();

//@Component(order=1)
struct CDamageCollider
{
    typedef i32 DamageWorldTeam;
    DamageWorldTeam team;
    DamageCore core;
    Collider collider;
    Array<DamageEvent> dmgEventQueue = {};
};

void updateDamageCollider(struct EntityComponentSystem* ecs, CDamageCollider* eltList, const i32 count,
                          const i32* entityId, f64 delta, f64 physLerpAlpha);

void onDeleteDamageCollider(struct EntityComponentSystem* ecs, CDamageCollider* eltList, const i32 count,
                            const i32* entityId, bool8* entDeleteFlag);
