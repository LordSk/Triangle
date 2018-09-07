#include <bx/math.h>
#include "player_ship.h"
#include "collision.h"
#include "input_recorder.h"
#include "ecs.h"
#include "renderer.h"
#include "game.h"

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
    PhysWorld& physWorld = getPhysWorld();

    for(i32 i = 0; i < count; i++) {
        CPlayerShipMovement& psm = eltList[i];
        const i32 eid = entityId[i];
        assert(ecs->entityCompBits[eid] & ComponentBit::Transform);
        assert(ecs->entityCompBits[eid] & ComponentBit::PhysBody);
        assert(ecs->entityCompBits[eid] & ComponentBit::ShipInput);

        CTransform& tf = ecs->getCompTransform(eid);
        CPhysBody& cpb = ecs->getCompPhysBody(eid);
        PhysBody& physBody = physWorld.bodyDyn[cpb.bodyId];
        CShipInput& input = ecs->getCompShipInput(eid);

        // compute mouse xy plane position
        // TODO: move this probably
        mat4 invView;
        mat4 viewProj;
        mat4 mtxPlayerView;
        const Camera& camPlayerView = rdr.cameraList[CameraID::PLAYER_VIEW];
        bx::mtxLookAtRh(mtxPlayerView, camPlayerView.eye, camPlayerView.at, camPlayerView.up);

        bx::mtxMul(viewProj, mtxPlayerView, rdr.mtxProj);
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


    // TODO: move
    for(i32 i = 0; i < count; i++) {
        const i32 eid = entityId[i];

        assert(ecs->entityCompBits[eid] & ComponentBit::PhysBody);
        assert(ecs->entityCompBits[eid] & ComponentBit::DmgZone);
        CDmgZone& dmgZone = ecs->getCompDmgZone(eid);
        CPhysBody& cpb = ecs->getCompPhysBody(eid);
        PhysBody& physBody = physWorld.bodyDyn[cpb.bodyId];

        const i32 interCount = dmgZone.lastFrameInterList.count;
        for(i32 j = 0; j < interCount; j++) {
            /*LOG("+-- %d %lld | %d %lld",
                dmgZone.lastFrameInterList[j].team1,
                (intptr_t)dmgZone.lastFrameInterList[j].zone1.data,
                dmgZone.lastFrameInterList[j].team2,
                (intptr_t)dmgZone.lastFrameInterList[j].zone2.data
                );*/
            physBody.vel -= vec2Norm(dmgZone.lastFrameInterList[j].collisionInfo.penVec) * 5;
        }
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

        assert(weap.hMeshBullet != MESH_HANDLE_INVALID);
        assert(ecs->entityCompBits[eid] & ComponentBit::Transform);
        assert(ecs->entityCompBits[eid] & ComponentBit::ShipInput);

        const bool8 inputFire = ecs->getCompShipInput(eid).fire;

        weap.fireCd -= delta;
        if(weap.fireCd <= 0.0 && inputFire) {
            weap.fireCd = 1.0f / weap.rateOfFire;

            // bullet entity
            const i32 bid = ecs->createEntity("Bullet");
            CTransform& bulletTf = ecs->addCompTransform(bid);
            CBulletMovement& bm = ecs->addCompBulletMovement(bid);
            CDmgZone& dmgBody = ecs->addCompDmgZone(bid);
            CDrawMesh& meshComp = ecs->addCompDrawMesh(bid);
            CBulletLogic& bulletLogic = ecs->addCompBulletLogic(bid);
            CLightPoint& bulletLight =  ecs->addCompLightPoint(bid);

            CTransform& weapTf = ecs->getCompTransform(eid); // Note: re-get the component since we added
            // a Transform and then might have realloced ArraySparse data

            bulletTf.pos = weapTf.pos;
            bulletTf.rot = weapTf.rot;

            Collider bulletCol;
            bulletCol.makeCb(CircleBound{{}, 0.7f});
            dmgBody.collider = bulletCol;
            dmgBody.team = weap.dmgTeam;
            dmgBody.tag = bid;

            // hacky to get direction
            // TODO: do better
            vec3 dx1 = {1, 0, 0};
            bx::vec3MulQuat(dx1, dx1, weapTf.rot);
            const vec2 dir = vec2Norm(vec3ToVec2(dx1));

            bm.vel = dir * weap.bulletSpeed;
            assert(bm.vel.x == bm.vel.x);
            assert(bm.vel.y == bm.vel.y);

            meshComp.hMesh = weap.hMeshBullet;
            meshComp.color = weap.meshColor;
            meshComp.unlit = true;

            quat baseRot;
            bx::quatRotateX(baseRot, -bx::kPiHalf);
            quat rotZ;
            bx::quatRotateZ(rotZ, -bx::kPiHalf);
            bx::quatMul(meshComp.tf.rot, baseRot, rotZ);
            meshComp.tf.scale = vec3Splat(0.5);

            vec3 color = vec4ToVec3(weap.meshColor);
            bulletLight.color1 = vec3Norm(color + vec3Splat(0.2));
            bulletLight.color2 = color;
            bulletLight.radius = 10.f;
            bulletLight.slope = 0;
            bulletLight.falloff = 5;
        }
    }
}

void onDeleteShipWeapon(EntityComponentSystem* ecs, CShipWeapon* eltList, const i32 count,
                        const i32* entityId, bool8* entDeleteFlag)
{
}
