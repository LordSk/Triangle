#include "damage.h"
#include "ecs.h"


DamageFrame::DamageFrame()
{
    intersectList.reserve(2048);
    for(i32 t = 0; t < (i32)DamageTeam::_COUNT; ++t) {
        colliders[t].reserve(512);
        payloads[t].reserve(512);
    }
}

void DamageFrame::registerCollider(const DamageTeam::Enum team, const Collider& c, const DamageCore& payload)
{
    assert(team >= 0 && team < DamageTeam::_COUNT);
    colliders[(i32)team].push(c);
    payloads[(i32)team].push(payload);
}

void DamageFrame::clearFrame()
{
    for(i32 t = 0; t < (i32)DamageTeam::_COUNT; ++t) {
        colliders[t].clear();
        payloads[t].clear();
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
                        intersInfo.pl1 = payloads[t1][c1];
                        intersInfo.pl1_cid = c1;
                        intersInfo.pl2 = payloads[t2][c2];
                        intersInfo.pl2_cid = c2;
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
                if(a.pl1_cid > b.pl1_cid) return 1;
                if(a.pl1_cid < b.pl1_cid) return -1;
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

void updateDamageCollider(EntityComponentSystem* ecs, CDamageCollider* eltList, const i32 count,
                          const i32* entityId, f64 delta, f64 physLerpAlpha)
{
    DamageFrame& dmgWorld = getDmgFrame();

    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        CDamageCollider& dmgBody = eltList[i];
        assert(ecs->entityCompBits[eid] & ComponentBit::Transform);
        CTransform& tf = ecs->getCompTransform(eid);
        dmgBody.collider.setPos(vec3ToVec2(tf.pos));
    }

    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        CDamageCollider& dmgBody = eltList[i];
        DamageCore zi = dmgBody.core;
        zi._entityUID = ecs->entityUID[eid];
        zi._eid = eid;
        dmgWorld.registerCollider((DamageTeam::Enum)dmgBody.team, dmgBody.collider, zi);
    }

    dmgWorld.resolveIntersections();

    DamageFrame::IntersectInfo* lastFrameInterList = dmgWorld.intersectList.data();
    const i32 lastFrameInterCount = dmgWorld.intersectList.count();

    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        CDamageCollider& dmgBody = eltList[i];
        const DamageFrame::IntersectInfo* start = nullptr;

        for(i32 j = 0; j < lastFrameInterCount; j++) {
            const DamageFrame::IntersectInfo& info = lastFrameInterList[j];

            if(info.team1 == dmgBody.team && info.pl1._eid == eid) {
                if(start == nullptr) {
                    start = &info;
                }

                DamageEvent event;
                event.collisionInfo = info.collisionInfo;
                event.flags = info.pl2.flags;
                event.dmg = info.pl2.dmg;
                event.fromEid = info.pl2._eid;
                event.fromEntityUID = info.pl2._entityUID;
                dmgBody.dmgEventQueue.push(event);
            }
            else if(start) { // end of the block
                break;
            }
        }

        assert(dmgBody.dmgEventQueue.count() < 1024);
    }
}

void onDeleteDamageCollider(EntityComponentSystem* ecs, CDamageCollider* eltList,
                            const i32 count, const i32* entityId, bool8* entDeleteFlag)
{
}
