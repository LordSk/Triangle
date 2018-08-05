#include "base.h"
#include "game.h"
#include "renderer.h"
#include "dbg_draw.h"
#include "collision.h"
#include <imgui/imgui.h>

DamageWorld::DamageWorld()
{
    for(i32 t = 0; t < (i32)Team::_COUNT; ++t) {
        colliders[t].reserve(512);
        zoneInfos[t].reserve(512);
    }
}

void DamageWorld::registerZone(const DamageWorld::Team team, Collider c, ZoneInfo zoneInfo)
{
    assert(team >= 0 && team < Team::_COUNT);
    colliders[(i32)team].push(c);
    zoneInfos[(i32)team].push(zoneInfo);
}

void DamageWorld::clearZones()
{
    for(i32 t = 0; t < (i32)Team::_COUNT; ++t) {
        colliders[t].clear();
        zoneInfos[t].clear();
    }
}

void DamageWorld::resolveIntersections(Array<IntersectInfo>* intersectList)
{
    intersectList->clear();

    for(i32 t1 = 0; t1 < (i32)Team::_COUNT; ++t1) {
        const i32 colCount1 = colliders[t1].count();
        const Collider* colList1 = colliders[t1].data();

        for(i32 t2 = 0; t2 < (i32)Team::_COUNT; ++t2) {
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
                        intersInfo.team1 = t1;
                        intersInfo.team2 = t2;
                        intersInfo.zone1 = zoneInfos[t1][c1];
                        intersInfo.zone1._cid = c1;
                        intersInfo.zone2 = zoneInfos[t2][c2];
                        intersInfo.zone2._cid = c2;
                        intersectList->push(intersInfo);
                    }
                }
            }
        }
    }

    // remove duplicates
    qsort(intersectList->data(), intersectList->count(), sizeof(IntersectInfo),
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
    });
}

void DamageWorld::dbgDraw()
{
    const vec4 teamColor[] = {
        vec4{1, 1, 1, 0.5f},
        vec4{0, 1, 0, 0.5f},
        vec4{1, 0, 0, 0.5f},
    };

    for(i32 t1 = 0; t1 < (i32)Team::_COUNT; ++t1) {
        const i32 colCount1 = colliders[t1].count();
        const Collider* colList1 = colliders[t1].data();
        for(i32 c1 = 0; c1 < colCount1; c1++) {
            const Collider& col1 = colList1[c1];
            colliderDbgDraw(col1, teamColor[t1]);
        }
    }
}


// TODO: remove this
static bx::FileReader g_fileReader;
static bx::FileWriter g_fileWriter;

void CameraFreeFlight::handleEvent(const SDL_Event& event)
{
    if(event.type == SDL_KEYDOWN && event.key.repeat == 0) {
        switch(event.key.keysym.sym) {
            case SDLK_z: input.forward = true; break;
            case SDLK_s: input.backward = true; break;
            case SDLK_q: input.left = true; break;
            case SDLK_d: input.right = true; break;
            case SDLK_a: input.up = true; break;
            case SDLK_e: input.down = true; break;
        }
        return;
    }
    if(event.type == SDL_KEYUP) {
        switch(event.key.keysym.sym) {
            case SDLK_z: input.forward = false; break;
            case SDLK_s: input.backward = false; break;
            case SDLK_q: input.left = false; break;
            case SDLK_d: input.right = false; break;
            case SDLK_a: input.up = false; break;
            case SDLK_e: input.down = false; break;
        }
        return;
    }

    if(event.type == SDL_MOUSEMOTION) {
        // TODO: move this to applyInput
        yaw = 0;
        pitch = 0;
        yaw += event.motion.xrel / (bx::kPi2 * 100.0);
        pitch += event.motion.yrel / (bx::kPi2 * 100.0);

        vec3 right;
        bx::vec3Cross(right, dir, up);
        quat qyaw;
        quat qpitch;
        bx::quatRotateAxis(qyaw, up, yaw);
        bx::quatRotateAxis(qpitch, right, pitch);
        bx::quatMul(rot, qyaw, qpitch);
        bx::quatNorm(rot, rot);
        //dir = {1, 0, 0};
        bx::vec3MulQuat(dir, dir, rot);
        bx::vec3Norm(dir, dir);
        return;
    }
}

void Room::make(vec3 size_, const i32 cubeSize)
{
    i32 sizexi = bx::ceil(size_.x);
    i32 sizeyi = bx::ceil(size_.y);
    size.x = sizexi + (cubeSize - (sizexi % cubeSize));
    size.y = sizeyi + (cubeSize - (sizeyi % cubeSize));
    size.z = size_.z;
    i32 width =  size.x / cubeSize;
    i32 height = size.y / cubeSize;

    cubeTransforms.clear();
    cubeTransforms.reserve(width * height + 4);

    // background
    for(i32 y = 0; y < height; y++) {
        for(i32 x = 0; x < width; x++) {
            Transform tfCube;
            tfCube.pos.x = cubeSize * 0.5 + x * cubeSize;
            tfCube.pos.y = cubeSize * 0.5 + y * cubeSize;
            tfCube.pos.z = -size.z + rand01() * 0.5;
            tfCube.scale = {(f32)cubeSize, (f32)cubeSize, 1.f};
            bx::quatIdentity(tfCube.rot);
            cubeTransforms.push_back(tfCube);
        }
    }

    // walls
    Transform tfWall;
    tfWall.scale = {size.x, cubeSize * 0.5f, size.z};
    tfWall.pos.x = size.x * 0.5f;
    tfWall.pos.y = -tfWall.scale.y * 0.5f;
    tfWall.pos.z = -size.z * 0.5;
    cubeTransforms.push_back(tfWall);

    tfWall = {};
    tfWall.scale = {size.x, cubeSize * 0.5f, size.z};
    tfWall.pos.x = size.x * 0.5f;
    tfWall.pos.y = size.y + tfWall.scale.y * 0.5f;
    tfWall.pos.z = -size.z * 0.5;
    cubeTransforms.push_back(tfWall);

    tfWall = {};
    tfWall.scale = {cubeSize * 0.5f, size.y, size.z};
    tfWall.pos.x = -tfWall.scale.x * 0.5f;
    tfWall.pos.y = size.y * 0.5;
    tfWall.pos.z = -size.z * 0.5;
    cubeTransforms.push_back(tfWall);

    tfWall = {};
    tfWall.scale = {cubeSize * 0.5f, size.y, size.z};
    tfWall.pos.x = size.x + tfWall.scale.x * 0.5f;
    tfWall.pos.y = size.y * 0.5;
    tfWall.pos.z = -size.z * 0.5;
    cubeTransforms.push_back(tfWall);

    Collider colWall;
    OrientedBoundingBox obbWall;
    obbWall.origin = vec2{ 0, -cubeSize * 0.5f };
    obbWall.size = vec2{ size.x, cubeSize * 0.5f };
    obbWall.angle = 0;
    physWorld.addStaticCollider(colWall.makeObb(obbWall));

    obbWall.origin = vec2{ 0, size.y };
    obbWall.size = vec2{ size.x, cubeSize * 0.5f };
    physWorld.addStaticCollider(colWall.makeObb(obbWall));

    obbWall.origin = vec2{ -cubeSize * 0.5f, 0 };
    obbWall.size = vec2{ cubeSize * 0.5f, size.y };
    physWorld.addStaticCollider(colWall.makeObb(obbWall));

    obbWall.origin = vec2{ size.x, 0 };
    obbWall.size = vec2{ cubeSize * 0.5f, size.y };
    physWorld.addStaticCollider(colWall.makeObb(obbWall));
}

void Room::dbgDrawPhysWorld()
{
    physWorld.dbgDraw(tfRoom.pos);
}

bool GameData::init()
{
    if(!(meshPlayerShip = meshLoad("assets/player_ship.mesh", &g_fileReader))) {
        return false;
    }

    if(!(meshEyeEn1 = meshLoad("assets/eye_enemy1.mesh", &g_fileReader))) {
        return false;
    }

    room.make(vec3{100, 50, 10}, 4);
    room.tfRoom.pos.z = 3;

    // create player entity
    playerEid = ecs.createEntity();
    CTransform& playerTf = ecs.addCompTransform(playerEid);
    CPhysBody& playerPhysBody = ecs.addCompPhysBody(playerEid);
    CDmgZone& playerDmgBody = ecs.addCompDmgZone(playerEid);
    CDrawMesh& playerMesh = ecs.addCompDrawMesh(playerEid);
    CShipInput& playerInput = ecs.addCompShipInput(playerEid);
    CShipControllerHuman& playerController = ecs.addCompShipControllerHuman(playerEid);
    CPlayerShipMovement& playerMovt = ecs.addCompPlayerShipMovement(playerEid);
    CShipWeapon& playerWeapon = ecs.addCompShipWeapon(playerEid);

    playerTf.pos = {10, 10, 0};
    playerTf.scale = {0.5f, 0.5f, 0.5f};

    PhysBody body{};
    Collider colPlayer;
    colPlayer.makeCb(CircleBound{ vec2{0, 0}, 1.2f });
    body.pos = vec2{ 10, 10 };
    body.weight = 1.0;
    body.bounceStrength = 0.0;
    room.physWorld.addDynamicBody(colPlayer, body, &playerPhysBody.bodyId);
    playerPhysBody.world = &room.physWorld;

    colPlayer.makeCb(CircleBound{ vec2{0, 0}, 1.0f });
    playerDmgBody.collider = colPlayer;
    playerDmgBody.team = DamageWorld::PLAYER;

    playerMesh.color = {1.0f, 0, 0, 1.0f};
    playerMesh.hMesh = meshPlayerShip;

    quat baseRot;
    bx::quatRotateX(baseRot, -bx::kPiHalf);
    quat rotZ;
    bx::quatRotateZ(rotZ, -bx::kPiHalf);
    bx::quatMul(playerMesh.tf.rot, baseRot, rotZ);

    playerWeapon.dmgTeam = DamageWorld::PLAYER;
    playerWeapon.rateOfFire = 10;
    // --------------------


    // create basic enemies
    for(i32 i = 0; i < 15; ++i) {
        const i32 eid = ecs.createEntity();
        CTransform& tf = ecs.addCompTransform(eid);
        CPhysBody& physBody = ecs.addCompPhysBody(eid);
        CDmgZone& dmgBody = ecs.addCompDmgZone(eid);
        CDrawMesh& mesh = ecs.addCompDrawMesh(eid);
        ecs.addCompShipInput(eid);
        ecs.addCompShipControllerAi(eid);
        ecs.addCompEnemyBasicMovement(eid);
        CShipWeapon& weap = ecs.addCompShipWeapon(eid);

        tf.pos = vec3{ (f32)randRange(10, room.size.x-10), (f32)randRange(10, room.size.y-10), 0 };
        tf.scale = vec3Splat(1.5);

        PhysBody body{};
        Collider collider;
        collider.makeCb(CircleBound{ vec2{0, 0}, 1.5f });
        body.pos = vec3ToVec2(tf.pos);
        body.weight = 1.0;
        body.bounceStrength = 0.0;
        room.physWorld.addDynamicBody(collider, body, &physBody.bodyId);
        physBody.world = &room.physWorld;

        dmgBody.collider = collider;
        dmgBody.team = DamageWorld::ENEMY;

        mesh.color = {(f32)rand01(), (f32)rand01(), (f32)rand01(), 1.0f};
        mesh.hMesh = meshEyeEn1;

        quat baseRot;
        bx::quatRotateX(baseRot, -bx::kPiHalf);
        quat rotZ;
        bx::quatRotateZ(rotZ, -bx::kPiHalf);
        bx::quatMul(mesh.tf.rot, baseRot, rotZ);

        weap.rateOfFire = 2.0f;
        weap.dmgTeam = DamageWorld::ENEMY;
    }

    return true;
}

void GameData::deinit()
{
    meshUnload(meshPlayerShip);
    meshUnload(meshEyeEn1);
}

void GameData::handlEvent(const SDL_Event& event)
{
    if(event.type == SDL_KEYDOWN && event.key.repeat == 0) {
        if(event.key.keysym.sym == SDLK_c) {
            cameraId += 1;
            cameraId = cameraId % 2;
            return;
        }
        if(event.key.keysym.sym == SDLK_b) {
            dbgEnableDmgZones ^= 1;
            return;
        }
    }

    if(mouseCaptured) {
        if(cameraId == CameraID::FREE_VIEW) {
            camFree.handleEvent(event);
        }
    }
}

void GameData::update(f64 delta)
{
    Renderer& rdr = getRenderer();
    time += delta;

    physWorldTimeAcc += delta;
    if(physWorldTimeAcc >= PHYS_UPDATE_DELTA) {
#ifdef CONF_DEBUG
        room.physWorld.update(PHYS_UPDATE_DELTA, 2);
#else
        room.physWorld.update(PHYS_UPDATE_DELTA, 10);
#endif
        physWorldTimeAcc = 0;
    }

    const f64 physLerpAlpha = physWorldTimeAcc / PHYS_UPDATE_DELTA;

    // set camera view
    mat4 mtxProj, mtxView;
    const vec3 up = { 0.0f, 0.0f, 1.0f };
    bx::mtxProjRh(mtxProj, 60.0f, f32(rdr.renderWidth)/f32(rdr.renderHeight), 0.1f, 1000.0f,
                  bgfx::getCaps()->homogeneousDepth);

    if(cameraId == CameraID::FREE_VIEW) {
        vec3 at;
        camFree.applyInput(delta);

        bx::vec3Add(at, camFree.pos, camFree.dir);
        bx::mtxLookAtRh(mtxView, camFree.pos, at, up);
    }
    else if(cameraId == CameraID::PLAYER_VIEW) {
        CameraBirdView camBv;
        camBv.pos = ecs.getCompTransform(playerEid).pos;
        camBv.height = dbgPlayerCamHeight;
        camBv.compute(&mtxView);
    }

    rdr.setView(mtxProj, mtxView);


    ecs.update(delta, physLerpAlpha);
    ecs.removeFlaggedForDeletion();

    dmgWorld.clearZones();

    for(i32 i = 0; i < ecs.comp_DmgZone.count(); ++i) {
        const CDmgZone& dmgBody = ecs.comp_DmgZone.data()[i];
        dmgWorld.registerZone((DamageWorld::Team)dmgBody.team, dmgBody.collider, {});
    }

    if(dbgEnableDmgZones) {
       dmgWorld.dbgDraw();
    }

    static Array<DamageWorld::IntersectInfo> intersectList;
    intersectList.reserve(2048);
    dmgWorld.resolveIntersections(&intersectList);


    ImGui::Begin("EntityComponentSystem");

    i32 entityCount = 0;
    for(i32 i = 0; i < MAX_ENTITIES; i++) {
        entityCount += ecs.entityCompBits[i] != 0;
    }

    ImGui::Text("Entity count (%d/%d):", entityCount, MAX_ENTITIES);
    ImGui::ProgressBar((f32)entityCount/MAX_ENTITIES);

    ImGui::End();
}

void GameData::render()
{
    Renderer& rdr = getRenderer();

    // debug room
#if 1
    mat4 mtxRoom;
    room.tfRoom.toMtx(&mtxRoom);

    const i32 cubeCount = room.cubeTransforms.size();
    static Array<InstanceData> instData;
    if(cubeCount > instData.capacity()) {
        instData.resize(cubeCount);
    }

    for(u32 i = 0; i < cubeCount; ++i) {
        InstanceData inst;
        Transform tf = room.cubeTransforms[i];
        mat4 mtx1;

        tf.pos.z = tf.pos.z - sin(time + i*0.21f) * 0.2;

        // model * roomModel
        tf.toMtx(&mtx1);
        bx::mtxMul(inst.mtxModel, mtx1, mtxRoom);

        inst.color = vec4{0.4f, 0.2f, 0.1f, 1.0f};
        instData[i] = inst;
    }

    rdr.drawCubeInstances(instData.data(), cubeCount);
#endif

    // TODO: temporary
    // mouse cursor
    if(cameraId == CameraID::PLAYER_VIEW) {
        Transform tfCursor;
        const vec3 curPos = ecs.getCompPlayerShipMovement(playerEid).curXyPlanePos;
        tfCursor.pos = curPos - vec3 {0.2f, 0.2f, 0.2f};
        tfCursor.scale = { 0.4f, 0.4f, 0.4f };
        dbgDrawRect(tfCursor, vec4{1, 0, 1, 1});
    }

    if(rdr.dbgEnableDbgPhysics) {
        room.dbgDrawPhysWorld();
    }
}
