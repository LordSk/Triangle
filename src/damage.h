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

struct DamageFrame
{
    struct ZoneInfo {
        u32 _cid; // do not set this variable
        u32 tag;
        void* data;
    };

    struct IntersectInfo {
        DamageTeam::Enum team1;
        DamageTeam::Enum team2;
        ZoneInfo zone1;
        ZoneInfo zone2;
        CollisionInfo collisionInfo;
    };


    // TODO: remove teams? accelerate search with a grid
    Array<Collider> colliders[DamageTeam::_COUNT];
    Array<ZoneInfo> zoneInfos[DamageTeam::_COUNT];
    Array<IntersectInfo> intersectList;

    DamageFrame();
    // done post physical world update
    void registerZone(const DamageTeam::Enum team, Collider c, ZoneInfo zoneInfo);
    void clearZones();
    void resolveIntersections();
    void dbgDraw();
};

void dmgFrameInit();
DamageFrame& getDmgFrame();
