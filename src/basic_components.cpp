#include "basic_components.h"
#include "ecs.h"
#include "renderer.h"
#include "input_recorder.h"

void updateTransform(EntityComponentSystem* ecs, CTransform* eltList, const i32 count, const i32* entityId,
                     f64 delta, f64 physLerpAlpha)
{
}

void updatePhysBody(EntityComponentSystem* ecs, CPhysBody* eltList, const i32 count, const i32* entityId,
                    f64 delta, f64 physLerpAlpha)
{
    PhysWorld& physWorld = getPhysWorld();

    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        const PhysBody& body = physWorld.bodyDyn[eltList[i].bodyId];
        assert(ecs->entityCompBits[eid] & ComponentBit::Transform);

        CTransform& tf = ecs->getCompTransform(eid);
        vec2 pos2 = vec2Lerp(body.prevPos, body.pos, physLerpAlpha);
        tf.pos.x = pos2.x;
        tf.pos.y = pos2.y;
    }
}

void updateDmgZone(EntityComponentSystem* ecs, CDmgZone* eltList, const i32 count, const i32* entityId,
                   f64 delta, f64 physLerpAlpha)
{
    DamageFrame& dmgWorld = getDmgFrame();

    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        CDmgZone& dmgBody = eltList[i];
        assert(ecs->entityCompBits[eid] & ComponentBit::Transform);
        CTransform& tf = ecs->getCompTransform(eid);
        dmgBody.collider.setPos(vec3ToVec2(tf.pos));
    }

    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        CDmgZone& dmgBody = eltList[i];
        DamageFrame::ZoneInfo zi;
        zi.tag = dmgBody.tag;
        zi.data = (void*)(intptr_t)eid;
        dmgWorld.registerZone((DamageTeam::Enum)dmgBody.team, dmgBody.collider, zi);
    }

    dmgWorld.resolveIntersections();

    DamageFrame::IntersectInfo* lastFrameInterList = dmgWorld.intersectList.data();
    const i32 lastFrameInterCount = dmgWorld.intersectList.count();

    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        CDmgZone& dmgBody = eltList[i];
        dmgBody.lastFrameInterList = {};

        for(i32 j = 0; j < lastFrameInterCount; j++) {
            const DamageFrame::IntersectInfo& info = lastFrameInterList[j];
            if(info.team1 == dmgBody.team && (intptr_t)info.zone1.data == eid) {
                if(dmgBody.lastFrameInterList.data == nullptr) {
                    dmgBody.lastFrameInterList.data = &lastFrameInterList[j];
                    dmgBody.lastFrameInterList.count = 1;
                }
                else {
                    dmgBody.lastFrameInterList.count++;
                }
            }
            else if(dmgBody.lastFrameInterList.data) {
                break;
            }
        }
    }
}

void updateEnemyBasicMovement(EntityComponentSystem* ecs, CEnemyBasicMovement* eltList, const i32 count,
                        const i32* entityId, f64 delta, f64 physLerpAlpha)
{
    const vec2 target = {50, 50};
    PhysWorld& physWorld = getPhysWorld();

    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        assert(ecs->entityCompBits[eid] & ComponentBit::Transform);
        assert(ecs->entityCompBits[eid] & ComponentBit::PhysBody);
        assert(ecs->entityCompBits[eid] & ComponentBit::ShipInput);

        CTransform& tf = ecs->getCompTransform(eid);
        CShipInput& input = ecs->getCompShipInput(eid);
        CPhysBody& cpb = ecs->getCompPhysBody(eid);
        PhysBody& physBody = physWorld.bodyDyn[cpb.bodyId];

        const vec2 pos2 = vec3ToVec2(tf.pos);
        const vec2 diff = target - pos2;

        // orient ship
        const f32 angle = atan2(-diff.y, diff.x);
        bx::quatRotateZ(tf.rot, angle);

        const vec2 right = vec2Norm(vec2Rotate(diff, bx::kPiHalf));
        const f32 rightDir = input.right - input.left;
        physBody.vel = right * (rightDir * 5.0f);

        const f32 fwdDir = (input.up - input.down);
        physBody.vel += vec2Norm(diff) * (fwdDir * 5.0f);
    }
}

void updateDrawMesh(EntityComponentSystem* ecs, CDrawMesh* eltList, const i32 count, const i32* entityId,
                    f64 delta, f64 physLerpAlpha)
{
    Renderer& rdr = getRenderer();

    // TODO: optimize this
    // TODO: move to render() function ?
    for(i32 i = 0; i < count; i++) {
        CDrawMesh& dm = eltList[i];
        assert(ecs->entityCompBits[entityId[i]] & ComponentBit::Transform);

        CTransform& baseTf = ecs->getCompTransform(entityId[i]);
        Transform finalTf = baseTf;
        finalTf.pos += dm.tf.pos;
        finalTf.scale = finalTf.scale * dm.tf.scale;
        bx::quatMul(finalTf.rot, dm.tf.rot,finalTf.rot);

        mat4 mtxModel;
        finalTf.toMtx(&mtxModel);
        rdr.drawMesh(dm.hMesh, mtxModel, dm.color);
    }
}

void updateBulletMovement(EntityComponentSystem* ecs, CBulletMovement* eltList, const i32 count,
                          const i32* entityId, f64 delta, f64 physLerpAlpha)
{
    for(i32 i = 0; i < count; i++) {
        CBulletMovement& bulletMove = eltList[i];
        const i32 eid = entityId[i];
        CTransform& tf = ecs->getCompTransform(eid);
        vec2 bpos = vec3ToVec2(tf.pos);
        bpos += bulletMove.vel * delta;
        tf.pos.x = bpos.x;
        tf.pos.y = bpos.y;

        if(bpos.x < -200 || bpos.x > 200 ||
           bpos.y < -200 || bpos.y > 200) {
            ecs->deleteEntity(eid);
        }
    }
}

void updateShipInput(EntityComponentSystem* ecs, CShipInput* eltList, const i32 count,
                               const i32* entityId, f64 delta, f64 physLerpAlpha)
{
}

void updateShipControllerHuman(EntityComponentSystem* ecs, CShipControllerHuman* eltList, const i32 count,
                               const i32* entityId, f64 delta, f64 physLerpAlpha)
{
    Renderer& rdr = getRenderer();
    if(rdr.currentCamId != 1) { // PLAYER_VIEW
        return;
    }

    // TODO: could be optimized...
    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        CShipInput& isc = ecs->getCompShipInput(eid);
        isc.up = inputIsKeyPressed(Vkey::Up);
        isc.down = inputIsKeyPressed(Vkey::Down);
        isc.left = inputIsKeyPressed(Vkey::Left);
        isc.right = inputIsKeyPressed(Vkey::Right);
        isc.fire = inputIsKeyPressed(Vkey::Fire);
        inputGetMousePos(&isc.mouseX, &isc.mouseY);
    }
}

void updateShipControllerAi(EntityComponentSystem* ecs, CShipControllerAi* eltList, const i32 count,
                            const i32* entityId, f64 delta, f64 physLerpAlpha)
{
    const vec2 target = {50, 50};

    for(i32 i = 0; i < count; i++) {
        CShipControllerAi& ai = eltList[i];
        const i32 eid = entityId[i];
        assert(ecs->entityCompBits[eid] & ComponentBit::Transform);
        assert(ecs->entityCompBits[eid] & ComponentBit::ShipInput);

        CTransform& tf = ecs->getCompTransform(eid);
        const vec2 pos2 = vec3ToVec2(tf.pos);
        const vec2 diff = target - pos2;

        CShipInput& input = ecs->getCompShipInput(eid);
        input.fire = true;

        ai.changeRightDirCd -= delta;
        ai.changeFwdDirCd -= delta;

        if(ai.changeRightDirCd <= 0.0) {
            ai.changeRightDirCd = randRange(1.0f, 2.0f);
            bool8 rightDir = xorshift32() & 1;
            input.left = !rightDir;
            input.right = rightDir;
        }

        if(ai.changeFwdDirCd <= 0.0) {
            ai.changeFwdDirCd = randRange(0.5f, 1.0f);

            const f32 dist = vec2Len(diff);
            i32 fwdDir = 1.0f;
            if(dist < 25) {
                fwdDir = (xorshift32() & 1) * 2 - 1;
            }
            if(dist < 10) {
                fwdDir = -1;
            }

            input.up = fwdDir == 1;
            input.down = fwdDir == -1;
        }
    }
}

void onDeleteTransform(EntityComponentSystem* ecs, CTransform* eltList, const i32 count,
                       const i32* entityId, bool8* entDeleteFlag)
{
}

void onDeletePhysBody(EntityComponentSystem* ecs, CPhysBody* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag)
{
    PhysWorld& physWorld = getPhysWorld();

    for(i32 i = 0; i < count; i++) {
        if(!entDeleteFlag[entityId[i]]) continue;
        CPhysBody& pb = eltList[i];
        assert(pb.bodyId >= 0);
        physWorld.removeBodyById(pb.bodyId);
    }
}

void onDeleteDmgZone(EntityComponentSystem* ecs, CDmgZone* eltList, const i32 count,
                     const i32* entityId, bool8* entDeleteFlag)
{
}

void onDeleteEnemyBasicMovement(EntityComponentSystem* ecs, CEnemyBasicMovement* eltList, const i32 count
                          ,const i32* entityId, bool8* entDeleteFlag)
{
}

void onDeleteDrawMesh(EntityComponentSystem* ecs, CDrawMesh* eltList, const i32 count,
                     const i32* entityId, bool8* entDeleteFlag)
{
}

void onDeleteBulletMovement(EntityComponentSystem* ecs, CBulletMovement* eltList, const i32 count,
                           const i32* entityId, bool8* entDeleteFlag)
{
}

void onDeleteShipInput(EntityComponentSystem* ecs, CShipInput* eltList, const i32 count,
                                const i32* entityId, bool8* entDeleteFlag)
{
}

void onDeleteShipControllerHuman(EntityComponentSystem* ecs, CShipControllerHuman* eltList, const i32 count, const i32* entityId, bool8* entDeleteFlag)
{
}

void onDeleteShipControllerAi(EntityComponentSystem* ecs, CShipControllerAi* eltList, const i32 count,
                              const i32* entityId, bool8* entDeleteFlag)
{
}

void updateHealthCore(EntityComponentSystem* ecs, CHealthCore* eltList, const i32 count, const i32* entityId,
                      f64 delta, f64 physLerpAlpha)
{
    for(i32 i = 0; i < count; i++) {
        CHealthCore& core = eltList[i];
        const i32 eid = entityId[i];

        if(ecs->entityCompBits[eid] & ComponentBit::DrawMesh) {
            CDrawMesh& mesh = ecs->getCompDrawMesh(eid);
            f32 alpha = mmax(0, core.health/core.healthMax);
            alpha *= alpha * alpha;
            mesh.color = vec4{0, alpha, 1, 1};
        }

        CDmgZone& dmgZone = ecs->getCompDmgZone(eid);
        const i32 interCount = dmgZone.lastFrameInterList.count;
        if(!interCount) continue;

        for(i32 j = 0; j < interCount; j++) {
            if(dmgZone.lastFrameInterList[j].team2 == DamageTeam::NEUTRAL) {
                continue;
            }
            core.health -= 10;
            if(core.health <= 0) {
                ecs->deleteEntity(eid);
            }
        }
    }
}

void onDeleteHealthCore(EntityComponentSystem* ecs, CHealthCore* eltList, const i32 count,
                        const i32* entityId, bool8* entDeleteFlag)
{

}

void updateBulletLogic(EntityComponentSystem* ecs, CBulletLogic* eltList, const i32 count,
                       const i32* entityId, f64 delta, f64 physLerpAlpha)
{
    for(i32 i = 0; i < count; i++) {
        CBulletLogic& bl = eltList[i];
        const i32 eid = entityId[i];
        CDmgZone& dmgZone = ecs->getCompDmgZone(eid);

        const i32 interCount = dmgZone.lastFrameInterList.count;
        for(i32 j = 0; j < interCount; j++) {
            ecs->deleteEntity(eid);
            break;
        }
    }
}

void onDeleteBulletLogic(EntityComponentSystem* ecs, CBulletLogic* eltList, const i32 count,
                         const i32* entityId, bool8* entDeleteFlag)
{

}

void updateLightPoint(EntityComponentSystem* ecs, CLightPoint* eltList, const i32 count, const i32* entityId,
                      f64 delta, f64 physLerpAlpha)
{
    Renderer& rdr = getRenderer();
    rdr.lightPointList.clear();

    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        CLightPoint& comp = eltList[i];

        LightPoint lp = comp;
        if(ecs->hasCompTransform(eid)) {
            CTransform& tf = ecs->getCompTransform(eid);
            lp.pos += tf.pos;
        }
        rdr.lightPointList.push(lp);
    }
}

void onDeleteLightPoint(EntityComponentSystem* ecs, CLightPoint* eltList, const i32 count,
                        const i32* entityId, bool8* entDeleteFlag)
{

}

void updateLightDirectional(EntityComponentSystem* ecs, CLightDirectional* eltList, const i32 count,
                            const i32* entityId, f64 delta, f64 physLerpAlpha)
{
    Renderer& rdr = getRenderer();
    rdr.lightDirectionalList.clear();

    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        CLightDirectional& comp = eltList[i];

        LightDirectional ld = comp;
        if(ecs->hasCompTransform(eid)) {
            CTransform& tf = ecs->getCompTransform(eid);
            ld.pos += tf.pos;
        }
        ld.dir = vec3Norm(comp.dir);
        rdr.lightDirectionalList.push(ld);
    }
}

void onDeleteLightDirectional(EntityComponentSystem* ecs, CLightDirectional* eltList, const i32 count,
                              const i32* entityId, bool8* entDeleteFlag)
{

}
