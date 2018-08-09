#include "base.h"
#include "game.h"
#include "renderer.h"
#include "dbg_draw.h"
#include "collision.h"
#include "damage.h"
#include <imgui/imgui.h>
#include <imgui/bgfx_imgui.h>

// TODO: remove this
static bx::FileReader g_fileReader;
static bx::FileWriter g_fileWriter;

void CameraBirdView::computeCamera(Camera* cam)
{
    bx::vec3Norm(dir, dir);
    cam->eye = pos + vec3{0, 0, height};
    cam->at = cam->eye + dir;
    cam->up = up;
}

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
    PhysWorld& physWorld = getPhysWorld();
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

bool GameData::init()
{
    physWorldInit();
    dmgFrameInit();

    if(!(meshPlayerShip = meshLoad("assets/player_ship.mesh", &g_fileReader))) {
        return false;
    }

    if(!(meshEyeEn1 = meshLoad("assets/eye_enemy1.mesh", &g_fileReader))) {
        return false;
    }

    if(!(meshBullet1 = meshLoad("assets/bullet_1.mesh", &g_fileReader))) {
        return false;
    }

    room.make(vec3{100, 50, 10}, 4);
    room.tfRoom.pos.z = 3;

    // create player entity
    playerEid = ecs.createEntity("PlayerShip");
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

    playerPhysBody.makeCircleBody(vec2{10, 10}, 1.2f, 1.0, 0.0);

    Collider colPlayer;
    colPlayer.makeCb(CircleBound{ vec2{0, 0}, 1.0f });
    playerDmgBody.collider = colPlayer;
    playerDmgBody.team = DamageTeam::PLAYER;

    playerMesh.color = {1.0f, 0, 0, 1.0f};
    playerMesh.hMesh = meshPlayerShip;

    quat baseRot;
    bx::quatRotateX(baseRot, -bx::kPiHalf);
    quat rotZ;
    bx::quatRotateZ(rotZ, -bx::kPiHalf);
    bx::quatMul(playerMesh.tf.rot, baseRot, rotZ);

    playerWeapon.dmgTeam = DamageTeam::PLAYER;
    playerWeapon.rateOfFire = 10;
    playerWeapon.hMeshBullet = meshBullet1;
    playerWeapon.meshColor = vec4{1, 0, 0, 1};
    // --------------------


    // create basic enemies
    for(i32 i = 0; i < 20; ++i) {
        const i32 eid = ecs.createEntity("BasicEnemy");
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


        physBody.makeCircleBody(vec3ToVec2(tf.pos), 1.5f, 1.0, 0.0);

        Collider collider;
        collider.makeCb(CircleBound{{}, 1.5f});
        dmgBody.collider = collider;
        dmgBody.team = DamageTeam::ENEMY;

        mesh.color = {(f32)rand01(), (f32)rand01(), (f32)rand01(), 1.0f};
        mesh.hMesh = meshEyeEn1;

        quat baseRot;
        bx::quatRotateX(baseRot, -bx::kPiHalf);
        quat rotZ;
        bx::quatRotateZ(rotZ, -bx::kPiHalf);
        bx::quatMul(mesh.tf.rot, baseRot, rotZ);

        weap.rateOfFire = 2.0f;
        weap.dmgTeam = DamageTeam::ENEMY;
        weap.hMeshBullet = meshBullet1;
        weap.meshColor = mesh.color;
    }

    return true;
}

void GameData::deinit()
{
    meshUnload(meshPlayerShip);
    meshUnload(meshEyeEn1);
    meshUnload(meshBullet1);
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
            camFreeView.handleEvent(event);
        }
    }
}

void GameData::componentDoUi(const i32 eid, const u64 compBit)
{
    switch(compBit) {
        case ComponentBit::Transform: {
            CTransform& tf = ecs.getCompTransform(eid);
            ImGui::InputFloat3("pos", tf.pos.data);
            ImGui::InputFloat3("scale", tf.scale.data);
            ImGui::InputFloat4("rot", tf.rot.data);
        } break;

        case ComponentBit::BulletMovement: {
            CBulletMovement& bm = ecs.getCompBulletMovement(eid);
            ImGui::InputFloat2("vel", bm.vel.data);
        } break;
    }
}

void GameData::update(f64 delta)
{
    Renderer& rdr = getRenderer();
    time += delta;

    PhysWorld& physWorld = getPhysWorld();

    physWorldTimeAcc += delta;
    if(physWorldTimeAcc >= PHYS_UPDATE_DELTA) {
#ifdef CONF_DEBUG
        physWorld.update(PHYS_UPDATE_DELTA, 2);
#else
        physWorld.update(PHYS_UPDATE_DELTA, 10);
#endif
        physWorldTimeAcc = 0;
    }

    const f64 physLerpAlpha = physWorldTimeAcc / PHYS_UPDATE_DELTA;

    DamageFrame& dmgWorld = getDmgFrame();
    dmgWorld.clearZones();

    ecs.removeFlaggedForDeletion();
    ecs.update(delta, physLerpAlpha);

    if(dbgEnableDmgZones) {
       dmgWorld.dbgDraw();
    }

    dmgWorld.resolveIntersections();

    // set camera view
    mat4 mtxProj;
    bx::mtxProjRh(mtxProj, 60.0f, f32(rdr.renderWidth)/f32(rdr.renderHeight), 0.1f, 1000.0f,
                  bgfx::getCaps()->homogeneousDepth);
    rdr.mtxProj = mtxProj;

    // set cameras
    if(cameraId == CameraID::FREE_VIEW) {
        camFreeView.applyInput(delta);
    }

    Camera _camFreeView;
    _camFreeView.eye = camFreeView.pos;
    _camFreeView.at = camFreeView.pos + camFreeView.dir;
    _camFreeView.up = camFreeView.up;
    rdr.setCamera(CameraID::FREE_VIEW, _camFreeView);

    Camera _camBirdView;
    CameraBirdView camBv;
    camBv.pos = ecs.getCompTransform(playerEid).pos;
    camBv.height = dbgPlayerCamHeight;
    camBv.computeCamera(&_camBirdView);
    rdr.setCamera(CameraID::PLAYER_VIEW, _camBirdView);

    if(cameraId == CameraID::FREE_VIEW) {
        rdr.selectCamera(CameraID::FREE_VIEW);
    }
    else if(cameraId == CameraID::PLAYER_VIEW) {
        rdr.selectCamera(CameraID::PLAYER_VIEW);
    }

    ImGui::Begin("EntityComponentSystem");

        i32 entityCount = 0;
        for(i32 i = 0; i < MAX_ENTITIES; i++) {
            entityCount += ecs.entityCompBits[i] != 0;
        }

        ImGui::Text("Entity count (%d/%d):", entityCount, MAX_ENTITIES);
        ImGui::ProgressBar((f32)entityCount/MAX_ENTITIES);

        ImGui::BeginChild("entity_list", ImVec2(-1, 0));
        for(i32 i = 0; i < MAX_ENTITIES; i++) {
            const u64 compBits = ecs.entityCompBits[i];

            if(compBits != 0) {
                char entityStr[64];
                sprintf(entityStr, "%s %d", ecs.entityName[i], i);
                if(ImGui::TreeNode(entityStr)) {
                    for(i32 c = 0; c < 64; c++) {
                        if(compBits & (u64(1) << c)) {
                            if(ImGui::TreeNode(ComponentBit::Names[c])) {
                                componentDoUi(i, (u64(1) << c));
                                ImGui::TreePop();
                            }
                        }
                    }
                    ImGui::TreePop();
                }
            }
        }
        ImGui::EndChild();

    ImGui::End();

    /*ImGui::Begin("Renderer");

    ImGui::Image(rdr.texShadowMap, ImVec2(512, 512));

    ImGui::End();*/
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

        //tf.pos.z = tf.pos.z - sin(time + i*0.21f) * 0.2;

        // model * roomModel
        tf.toMtx(&mtx1);
        bx::mtxMul(inst.mtxModel, mtx1, mtxRoom);

        inst.color = vec4{0.4f, 0.2f, 0.1f, 1.0f};
        instData[i] = inst;
    }

    rdr.drawCubeInstances(instData.data(), cubeCount, true);
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
        getPhysWorld().dbgDraw({0, 0, 0});
    }
}
