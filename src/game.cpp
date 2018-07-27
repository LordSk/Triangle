#include "base.h"
#include "game.h"
#include "renderer.h"
#include "dbg_draw.h"
#include "collision.h"

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

    /*
    Collider col;
    OrientedBoundingBox obb;
    obb.origin = vec2{10, 10};
    obb.size = vec2{10, 20};
    obb.angle = bx::kPiQuarter;
    room.physWorld.addStaticCollider(col.makeObb(obb));

    for(i32 i = 0; i < 100; i++) {
        Collider col;
        CircleBound cb;
        cb.center = vec2{f32(10 + rand01() * 90), f32(10 + rand01() * 40)};
        cb.radius = 1.0 + rand01() * 1;
        PhysBody ball;
        f32 a = rand01() * bx::kPi2;
        f32 speed = 10 + rand01() * 50;
        ball.vel = { cos(a) * speed, sin(a) * speed };
        ball.pos = cb.center;
        ball.bounceStrength = 1.0;
        room.physWorld.addDynamicBody(col.makeCb(cb), ball);
    }
    */

    playerShip.tf = &compTransform[compTransformCount++];
    playerShip.dmgBody = &compDmgBody[compDmgBodyCount++];


    playerShip.init();
    playerShip.tf->pos = {10, 10, 0};

    PhysBody body = {};
    Collider colPlayer;
    colPlayer.makeCb(CircleBound{ vec2{0, 0}, 1.2f });
    body.pos = vec2{ 10, 10 };
    body.weight = 1.0;
    body.bounceStrength = 0.0;
    playerShip.physBody = room.physWorld.addDynamicBody(colPlayer, body);

    playerShip.dmgBody->collider = colPlayer;
    playerShip.dmgBody->team = DamageWorld::PLAYER;

    // init enemy1 s
    const i32 en1Count = arr_count(enemy1List);
    for(i32 i = 0; i < en1Count; ++i) {
        Enemy1& en1 = enemy1List[i];
        assert(compTransformCount < arr_count(compTransform));
        assert(compDmgBodyCount < arr_count(compDmgBody));

        en1.tf = &compTransform[compTransformCount++];
        en1.dmgBody = &compDmgBody[compDmgBodyCount++];
        en1.tf->pos = vec3{ (f32)randRange(10, room.size.x-10), (f32)randRange(10, room.size.y-10), 0 };
        en1.tf->scale = vec3Splat(1.5);

        PhysBody body = {};
        Collider collider;
        collider.makeCb(CircleBound{ vec2{0, 0}, 1.5f });
        body.pos = vec3ToVec2(en1.tf->pos);
        body.weight = 1.0;
        body.bounceStrength = 0.0;
        en1.physBody = room.physWorld.addDynamicBody(collider, body);

        en1.dmgBody->collider = collider;
        en1.dmgBody->team = DamageWorld::ENEMY;

        en1.changeRightDirCd = 0.0;
        en1.changeFwdDirCd = 0.0;
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
    }

    if(mouseCaptured) {
        if(cameraId == CameraID::FREE_VIEW) {
            camFree.handleEvent(event);
        }
        else if(cameraId == CameraID::PLAYER_VIEW) {
            playerShip.handleEvent(event);
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

    // update player ship
    playerShip.update(delta, physLerpAlpha);
    playerShip.dmgBody->collider.setPos(vec3ToVec2(playerShip.tf->pos));

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
        camBv.pos = playerShip.tf->pos;
        camBv.height = dbgPlayerCamHeight;
        camBv.compute(&mtxView);

        mat4 invView;
        mat4 viewProj;
        bx::mtxMul(viewProj, mtxView, mtxProj);
        bx::mtxInverse(invView, viewProj);
        playerShip.computeCursorPos(invView, dbgPlayerCamHeight);
    }

    rdr.setView(mtxProj, mtxView);

    // update enemy1
    const i32 en1Count = arr_count(enemy1List);
    for(i32 i = 0; i < en1Count; ++i) {
        Enemy1& en1 = enemy1List[i];
        const vec2 pos2 = vec2Lerp(en1.physBody->prevPos, en1.physBody->pos, physLerpAlpha);
        en1.tf->pos = vec2ToVec3(pos2);
        en1.target = vec3ToVec2(playerShip.tf->pos);
        const vec2 diff = en1.target - pos2;

        en1.changeRightDirCd -= delta;
        en1.changeFwdDirCd -= delta;

        if(en1.changeRightDirCd <= 0.0) {
            en1.changeRightDirCd = randRange(1.0f, 2.0f);
            vec2 right = vec2Norm(vec2Rotate(diff, bx::kPiHalf));
            f32 rightDir = (xorshift32() & 1) * 2.0 - 1.0;
            en1.physBody->vel = right * (rightDir * 5.0f) + vec2Norm(diff) * randRange(-5.0f, 5.0f);
        }

        if(en1.changeFwdDirCd <= 0.0) {
            en1.changeFwdDirCd = randRange(0.5f, 1.0f);

            const f32 dist = vec2Len(diff);
            f32 fwdDir = 1.0f;
            if(dist < 25) {
                fwdDir = (xorshift32() & 1) * 2.0 - 1.0;
            }
            if(dist < 10) {
                fwdDir = -1.0f;
            }
            en1.physBody->vel += vec2Norm(diff) * (fwdDir * 5.0f);
        }

        en1.dmgBody->collider.setPos(pos2);
    }

    static f64 fireCd = 0;
    fireCd -= delta;
    if(fireCd <= 0.0 && playerShip.input.fire) {
        fireCd = 0.1;
        WeaponBullet bullet;

        Collider bulletCol;
        bulletCol.makeCb(CircleBound{vec2{0, 0}, 0.7f});
        assert(compDmgBodyCount < arr_count(compDmgBody));
        bullet.dmgBody = &compDmgBody[compDmgBodyCount++];
        bullet.dmgBody->collider = bulletCol;
        bullet.dmgBody->team = DamageWorld::PLAYER;
        bullet.pos = vec3ToVec2(playerShip.tf->pos);
        bullet.vel = vec2Norm(vec3ToVec2(playerShip.mousePosWorld - playerShip.tf->pos)) * 40.0f;
        assert(weapBulletCount < arr_count(weapBulletList));
        weapBulletList[weapBulletCount++] = bullet;
    }


    const i32 weapBulletCount2 = weapBulletCount;
    for(i32 i = 0; i < weapBulletCount2; i++) {
        WeaponBullet& wb = weapBulletList[i];
        wb.pos += wb.vel * delta;
        wb.dmgBody->collider.setPos(wb.pos);
    }

    dmgWorld.clearZones();

    dmgWorld.registerZone(DamageWorld::PLAYER, playerShip.dmgBody->collider, {});

    for(i32 i = 0; i < en1Count; ++i) {
        Enemy1& en1 = enemy1List[i];
        dmgWorld.registerZone(DamageWorld::ENEMY, en1.dmgBody->collider, {});
    }
    for(i32 i = 0; i < weapBulletCount2; i++) {
        WeaponBullet& wb = weapBulletList[i];
        dmgWorld.registerZone(DamageWorld::ENEMY, wb.dmgBody->collider, {});
    }

    dmgWorld.dbgDraw();

    static Array<DamageWorld::IntersectInfo> intersectList;
    intersectList.reserve(2048);
    dmgWorld.resolveIntersections(&intersectList);
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

    // draw enemy1
    const i32 en1Count = arr_count(enemy1List);
    for(i32 i = 0; i < en1Count; ++i) {
        Transform tf = *enemy1List[i].tf;

        quat baseRot;
        bx::quatRotateX(baseRot, -bx::kPiHalf);
        quat rotZ;
        bx::quatRotateZ(rotZ, -bx::kPiHalf);
        bx::quatMul(baseRot, baseRot, rotZ);

        const vec2 diff = enemy1List[i].target - vec3ToVec2(tf.pos);
        const f32 angle = atan2(-diff.y, diff.x);
        bx::quatRotateZ(rotZ, angle);
        bx::quatMul(tf.rot, baseRot, rotZ);

        mat4 mtxModelEyeEn;
        tf.toMtx(&mtxModelEyeEn);

        const vec4 bodyColor = {0.2f, 0, 1.0f, 1};
        rdr.drawMesh(meshEyeEn1, mtxModelEyeEn, bodyColor);
    }


    //player ship
    mat4 mtxModel;
    playerShip.tf->toMtx(&mtxModel);
    rdr.drawMesh(meshPlayerShip, mtxModel, vec4{1, 0, 0, 1});

    // mouse cursor
    if(cameraId == CameraID::PLAYER_VIEW) {
        Transform tfCursor;
        tfCursor.pos = playerShip.mousePosWorld - vec3 {0.2f, 0.2f, 0.2f};
        tfCursor.scale = { 0.4f, 0.4f, 0.4f };
        dbgDrawRect(tfCursor, vec4{1, 0, 1, 1});
    }

    if(rdr.dbgEnableDbgPhysics) {
        room.dbgDrawPhysWorld();
    }
}
