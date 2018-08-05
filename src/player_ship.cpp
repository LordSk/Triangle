#include <bx/math.h>
#include "player_ship.h"
#include "collision.h"
#include "input_recorder.h"
#include "ecs.h"
#include "renderer.h"
#include "game.h" // Damage* stuff, TODO: remove

static vec3 screenToXyPlanePos(const mat4& invViewProj, vec2 screenPos, vec3 camPos)
{
    // TODO: plug view size in
    const i32 WINDOW_WIDTH = 1600;
    const i32 WINDOW_HEIGHT = 900;

    vec4 screenPosNormNear = { screenPos.x / (WINDOW_WIDTH * 0.5f),
                              -screenPos.y / (WINDOW_HEIGHT * 0.5f), 0.0, 1.0 };
    vec4 screenPosNormFar  = { screenPos.x / (WINDOW_WIDTH * 0.5f),
                              -screenPos.y / (WINDOW_HEIGHT * 0.5f), 1.0, 1.0 };

    vec4 worldPosNear, worldPosFar;
    bx::vec4MulMtx(worldPosNear, screenPosNormNear, invViewProj);
    bx::vec4MulMtx(worldPosFar, screenPosNormFar, invViewProj);

    bx::vec3Mul(worldPosNear, worldPosNear, 1.0 / worldPosNear.w);
    bx::vec3Mul(worldPosFar, worldPosFar, 1.0 / worldPosFar.w);
    vec3 worldDir;
    bx::vec3Sub(worldDir, worldPosFar, worldPosNear);
    vec3 camDir;
    bx::vec3Norm(camDir, worldDir);

    vec3 eye = camPos;
    vec3 p0 = {0, 0, 0};
    vec3 pn = {0, 0, 1};

    vec3 xyPlanePos;
    planeLineIntersection(&xyPlanePos, eye, camDir, p0, pn);
    return xyPlanePos;
}

void updatePlayerShipMovement(EntityComponentSystem* ecs, CPlayerShipMovement* eltList, const i32 count,
                              const i32* entityId, f64 delta, f64 physLerpAlpha)
{
    Renderer& rdr = getRenderer();

    for(i32 i = 0; i < count; i++) {
        CPlayerShipMovement& psm = eltList[i];
        const i32 eid = entityId[i];
        assert(ecs->entityCompBits[eid] & ComponentBit::Transform);
        assert(ecs->entityCompBits[eid] & ComponentBit::PhysBody);
        assert(ecs->entityCompBits[eid] & ComponentBit::ShipInput);

        CTransform& tf = ecs->getCompTransform(eid);
        CPhysBody& cpb = ecs->getCompPhysBody(eid);
        PhysBody& physBody = cpb.world->bodyDyn[cpb.bodyId];
        CShipInput& input = ecs->getCompShipInput(eid);

        // compute mouse xy plane position
        // TODO: move this probably
        mat4 invView;
        mat4 viewProj;
        bx::mtxMul(viewProj, rdr.mtxView, rdr.mtxProj);
        bx::mtxInverse(invView, viewProj);
        const vec2 mousePos = { input.mouseX, input.mouseY };
        const vec3 xyPlanePos = screenToXyPlanePos(invView, mousePos, tf.pos + vec3{0, 0, 30});
        const vec2 mousePosWorld = vec3ToVec2(xyPlanePos);
        psm.curXyPlanePos = xyPlanePos;

        const vec2 pos2 = vec3ToVec2(tf.pos);

        const vec2 mouseDir = vec2Norm(vec2{ mousePosWorld.x - pos2.x, mousePosWorld.y - pos2.y });
        const f32 angle = bx::atan2(-mouseDir.y, mouseDir.x);

        vec2 dir;
        dir.x = (input.right - input.left);
        dir.y = (input.up - input.down);

//#define ADVANCED_MOVEMENT
#ifdef ADVANCED_MOVEMENT
        f32 accel = 100.0f;

        if(vec2Len(dir) > 0) {
            dir = vec2Norm(dir);
            accel += 200.0f * clamp(vec2Dot(mouseDir, dir), -0.2f, 1.0f);
            body->vel += dir * accel * delta;
        }
        else {
            // apply friction
            body->vel = body->vel * (1.0 - bx::clamp(20.0 * delta, 0.0, 1.0));
        }

        const f32 maxSpeed = 30.0f + 20.0f * clamp(vec2Dot(mouseDir, dir), 0.0f, 1.0f);
        if(vec2Len(body->vel) > maxSpeed) {
            body->vel = vec2Norm(body->vel) * maxSpeed;

#else
        if(vec2Len(dir) > 0) {
            dir = vec2Norm(dir);
            physBody.vel += dir * psm.accel * delta;
        }
        else {
            // apply friction
            physBody.vel = physBody.vel * (1.0 - bx::clamp(psm.deccel * delta, 0.0, 1.0));
        }

        const f32 maxSpeed = 40.0f;
        if(vec2Len(physBody.vel) > maxSpeed) {
            physBody.vel = vec2Norm(physBody.vel) * psm.maxSpeed;
        }
#endif

        bx::quatRotateZ(tf.rot, angle);
    }
}

void onDeletePlayerShipMovement(EntityComponentSystem* ecs, CPlayerShipMovement* eltList,
                                const i32 count, const i32* entityId, bool8* entDeleteFlag)
{

}

void updateShipWeapon(EntityComponentSystem* ecs, CShipWeapon* eltList, const i32 count,
                      const i32* entityId, f64 delta, f64 physLerpAlpha)
{
    for(i32 i = 0; i < count; i++) {
        CShipWeapon& weap = eltList[i];
        const i32 eid = entityId[i];

        assert(ecs->entityCompBits[eid] & ComponentBit::Transform);
        assert(ecs->entityCompBits[eid] & ComponentBit::ShipInput);

        const bool8 inputFire = ecs->getCompShipInput(eid).fire;

        weap.fireCd -= delta;
        if(weap.fireCd <= 0.0 && inputFire) {
            weap.fireCd = 1.0f / weap.rateOfFire;

            CTransform& tf = ecs->getCompTransform(eid);

            // bullet entity
            const i32 eid = ecs->createEntity();
            ecs->addCompTransform(eid);
            CBulletMovement& bm = ecs->addCompBulletMovement(eid);
            CDmgZone& dmgBody = ecs->addCompDmgZone(eid);

            Collider bulletCol;
            bulletCol.makeCb(CircleBound{vec2{0, 0}, 0.7f});
            dmgBody.collider = bulletCol;
            dmgBody.team = weap.dmgTeam;
            bm.pos = vec3ToVec2(tf.pos);

            // hacky to get direction
            // TODO: do better
            vec3 dx1 = {1, 0, 0};
            bx::vec3MulQuat(dx1, dx1, tf.rot);
            const vec2 dir = vec2Norm(vec3ToVec2(dx1));

            bm.vel = dir * 60.0f;
        }
    }
}

void onDeleteShipWeapon(EntityComponentSystem* ecs, CShipWeapon* eltList, const i32 count,
                        const i32* entityId, bool8* entDeleteFlag)
{
}
