#include "damage.h"


DamageFrame::DamageFrame()
{
    intersectList.reserve(2048);
    for(i32 t = 0; t < (i32)DamageTeam::_COUNT; ++t) {
        colliders[t].reserve(512);
        zoneInfos[t].reserve(512);
    }
}

void DamageFrame::registerZone(const DamageTeam::Enum team, Collider c, ZoneInfo zoneInfo)
{
    assert(team >= 0 && team < DamageTeam::_COUNT);
    colliders[(i32)team].push(c);
    zoneInfos[(i32)team].push(zoneInfo);
}

void DamageFrame::clearZones()
{
    for(i32 t = 0; t < (i32)DamageTeam::_COUNT; ++t) {
        colliders[t].clear();
        zoneInfos[t].clear();
    }
}

void DamageFrame::resolveIntersections()
{
    intersectList.clear();

    for(i32 t1 = 0; t1 < (i32)DamageTeam::_COUNT; ++t1) {
        const i32 colCount1 = colliders[t1].count();
        const Collider* colList1 = colliders[t1].data();

        for(i32 t2 = 0; t2 < (i32)DamageTeam::_COUNT; ++t2) {
            if(t1 == t2) continue;
            const i32 colCount2 = colliders[t2].count();
            const Collider* colList2 = colliders[t2].data();

            for(i32 c1 = 0; c1 < colCount1; c1++) {
                const Collider& col1 = colList1[c1];

                for(i32 c2 = 0; c2 < colCount2; c2++) {
                    const Collider& col2 = colList2[c2];

                    CollisionInfo coliInfo;
                    if(colliderIntersect(col1, col2, &coliInfo)) {
                        IntersectInfo intersInfo;
                        intersInfo.team1 = (DamageTeam::Enum)t1;
                        intersInfo.team2 = (DamageTeam::Enum)t2;
                        intersInfo.zone1 = zoneInfos[t1][c1];
                        intersInfo.zone1._cid = c1;
                        intersInfo.zone2 = zoneInfos[t2][c2];
                        intersInfo.zone2._cid = c2;
                        intersInfo.collisionInfo = coliInfo;
                        intersectList.push(intersInfo);
                    }
                }
            }
        }
    }

    // sort
    /*qsort(intersectList.data(), intersectList.count(), sizeof(IntersectInfo),
          [](const void* pa, const void* pb) {
                const IntersectInfo& a = *(IntersectInfo*)pa;
                const IntersectInfo& b = *(IntersectInfo*)pb;
                const u32 minCidA = mmin(a.zone1._cid, a.zone2._cid);
                const u32 maxCidA = mmax(a.zone1._cid, a.zone2._cid);
                const u32 minCidB = mmin(b.zone1._cid, b.zone2._cid);
                const u32 maxCidB = mmax(b.zone1._cid, b.zone2._cid);
                if(minCidA < minCidB) return -1;
                if(minCidA > minCidB) return 1;
                if(maxCidA < maxCidB) return -1;
                if(maxCidA > maxCidB) return 1;
                return 0;
    });*/

    qsort(intersectList.data(), intersectList.count(), sizeof(IntersectInfo),
          [](const void* pa, const void* pb) {
                const IntersectInfo& a = *(IntersectInfo*)pa;
                const IntersectInfo& b = *(IntersectInfo*)pb;
                if(a.team1 < b.team1) return -1;
                if(a.team1 > b.team1) return 1;
                if(a.zone1._cid > b.zone1._cid) return 1;
                if(a.zone1._cid < b.zone1._cid) return -1;
                return 0;
    });
}

void DamageFrame::dbgDraw()
{
    const vec4 teamColor[] = {
        vec4{1, 1, 1, 0.5f},
        vec4{0, 1, 0, 0.5f},
        vec4{1, 0, 0, 0.5f},
    };

    for(i32 t1 = 0; t1 < (i32)DamageTeam::_COUNT; ++t1) {
        const i32 colCount1 = colliders[t1].count();
        const Collider* colList1 = colliders[t1].data();
        for(i32 c1 = 0; c1 < colCount1; c1++) {
            const Collider& col1 = colList1[c1];
            colliderDbgDraw(col1, teamColor[t1]);
        }
    }
}

static DamageFrame* g_dmgFrame = nullptr;

DamageFrame& getDmgFrame()
{
    assert(g_dmgFrame);
    return *g_dmgFrame;
}

void dmgFrameInit()
{
    static DamageFrame dmg;
    g_dmgFrame = &dmg;
}
