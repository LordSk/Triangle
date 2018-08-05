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
    for(i32 i = 0; i < count; i++) {
        const PhysBody& body = eltList[i].world->bodyDyn[eltList[i].bodyId];
        assert(ecs->entityCompBits[entityId[i]] & ComponentBit::Transform);

        CTransform& tf = ecs->getCompTransform(entityId[i]);
        vec2 pos2 = vec2Lerp(body.prevPos, body.pos, physLerpAlpha);
        tf.pos.x = pos2.x;
        tf.pos.y = pos2.y;
    }
}

void updateDmgZone(EntityComponentSystem* ecs, CDmgZone* eltList, const i32 count, const i32* entityId,
                   f64 delta, f64 physLerpAlpha)
{
    for(i32 i = 0; i < count; i++) {
        CDmgZone& dmgBody = eltList[i];
        assert(ecs->entityCompBits[entityId[i]] & ComponentBit::Transform);

        CTransform& tf = ecs->getCompTransform(entityId[i]);
        dmgBody.collider.setPos(vec3ToVec2(tf.pos));
    }
}

void updateAiBasicEnemy(EntityComponentSystem* ecs, CAiBasicEnemy* eltList, const i32 count,
                        const i32* entityId, f64 delta, f64 physLerpAlpha)
{
    vec2 target = {50, 50};

    for(i32 i = 0; i < count; i++) {
        CAiBasicEnemy& ai = eltList[i];
        assert(ecs->entityCompBits[entityId[i]] & ComponentBit::Transform);
        assert(ecs->entityCompBits[entityId[i]] & ComponentBit::PhysBody);

        CTransform& tf = ecs->getCompTransform(entityId[i]);
        const vec2 pos2 = vec3ToVec2(tf.pos);
        const vec2 diff = target - pos2;

        // orient ship
        const f32 angle = atan2(-diff.y, diff.x);
        bx::quatRotateZ(tf.rot, angle);

        ai.changeRightDirCd -= delta;
        ai.changeFwdDirCd -= delta;

        if(ai.changeRightDirCd <= 0.0) {
            ai.changeRightDirCd = randRange(1.0f, 2.0f);
            CPhysBody& cpb = ecs->getCompPhysBody(entityId[i]);
            PhysBody& physBody = cpb.world->bodyDyn[cpb.bodyId];

            vec2 right = vec2Norm(vec2Rotate(diff, bx::kPiHalf));
            f32 rightDir = (xorshift32() & 1) * 2.0 - 1.0;
            physBody.vel = right * (rightDir * 5.0f) + vec2Norm(diff) * randRange(-5.0f, 5.0f);
        }

        if(ai.changeFwdDirCd <= 0.0) {
            ai.changeFwdDirCd = randRange(0.5f, 1.0f);
            CPhysBody& cpb = ecs->getCompPhysBody(entityId[i]);
            PhysBody& physBody = cpb.world->bodyDyn[cpb.bodyId];

            const f32 dist = vec2Len(diff);
            f32 fwdDir = 1.0f;
            if(dist < 25) {
                fwdDir = (xorshift32() & 1) * 2.0 - 1.0;
            }
            if(dist < 10) {
                fwdDir = -1.0f;
            }
            physBody.vel += vec2Norm(diff) * (fwdDir * 5.0f);
        }
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
        assert(ecs->entityCompBits[eid] & ComponentBit::Transform);
        CTransform& tf = ecs->getCompTransform(eid);
        vec2& bpos = bulletMove.pos;
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
    // TODO: could be optimized...
    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        assert(ecs->entityCompBits[eid] & ComponentBit::ShipInput);
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
    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];
        assert(ecs->entityCompBits[eid] & ComponentBit::ShipInput);
        CShipInput& isc = ecs->getCompShipInput(eid);
        isc.fire = true;
    }
}

void onDeleteTransform(EntityComponentSystem* ecs, CTransform* eltList, const i32 count,
                       const i32* entityId, bool8* entDeleteFlag)
{
}

void onDeletePhysBody(EntityComponentSystem* ecs, CPhysBody* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag)
{
    for(i32 i = 0; i < count; i++) {
        if(!entDeleteFlag[entityId[i]]) continue;
        CPhysBody& pb = eltList[i];
        assert(pb.world);
        assert(pb.bodyId >= 0);
        pb.world->removeBodyById(pb.bodyId);
    }
}

void onDeleteDmgZone(EntityComponentSystem* ecs, CDmgZone* eltList, const i32 count,
                     const i32* entityId, bool8* entDeleteFlag)
{
}

void onDeleteAiBasicEnemy(EntityComponentSystem* ecs, CAiBasicEnemy* eltList, const i32 count
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
