#pragma once

#include <SDL2/SDL_events.h>
#include "base.h"
#include "utils.h"
#include "vector_math.h"
#include "collision.h"
#include "player_ship.h"
#include "mesh_load.h"
#include "ecs.h"

struct DamageWorld
{
    struct ZoneInfo {
        u32 zid;
        u32 _cid; // do not set this variable
        void* data;
    };

    struct IntersectInfo {
        i32 team1;
        i32 team2;
        ZoneInfo zone1;
        ZoneInfo zone2;
    };

    enum Team {
        NEUTRAL=0,
        PLAYER,
        ENEMY,
        _COUNT
    };

    // TODO: remove teams? accelerate search with a grid
    Array<Collider> colliders[Team::_COUNT];
    Array<ZoneInfo> zoneInfos[Team::_COUNT];

    DamageWorld();
    // done post physical world update
    void registerZone(const Team team, Collider c, ZoneInfo zoneInfo);
    void clearZones();
    void resolveIntersections(Array<IntersectInfo>* intersectList);
    void dbgDraw();
};

struct CameraID {
    enum Enum {
        FREE_VIEW = 0,
        PLAYER_VIEW,
    };
};

struct CameraBirdView
{
    vec3 pos;
    vec3 dir = {0, 0.001f, -1.f};
    vec3 up = {0, 0, 1.f};
    f32 height;

    inline void compute(mat4* lookAt) {
        bx::vec3Norm(dir, dir);
        vec3 eye = pos + vec3{0, 0, height};
        vec3 at = eye + dir;
        bx::mtxLookAtRh(*lookAt, eye, at, up);
    }
};

struct CameraFreeFlight
{
    vec3 pos = {0, 0, 10};
    vec3 dir = {1, 0, 0};
    vec3 up = {0, 0, 1};
    quat rot;
    f32 speed = 30;

    struct {
        bool8 forward;
        bool8 backward;
        bool8 left;
        bool8 right;
        bool8 up;
        bool8 down;
    } input = {};

    f64 pitch = 0;
    f64 yaw = 0;

    void handleEvent(const SDL_Event& event);

    inline void applyInput(f64 delta) {
        vec3 right;
        bx::vec3Cross(right, dir, up);
        bx::vec3Norm(right, right);

        pos += dir * speed * delta * (input.forward - input.backward);
        pos += up * speed * delta * (input.up - input.down);
        pos += right * speed * delta * (input.right - input.left);
    }
};

struct Room
{
    Transform tfRoom;
    vec3 size;
    Array<Transform> cubeTransforms;
    Array<mat4> cubeTfMtx;
    PhysWorld physWorld; // TODO: move this out

    void make(vec3 size_, const i32 cubeSize);

    void dbgDrawPhysWorld();
};

struct GameData
{
    f64 time = 0;
    f64 physWorldTimeAcc = 0;
    i32 cameraId = CameraID::PLAYER_VIEW;

    bool mouseCaptured = true;
    CameraFreeFlight camFree;

    f32 dbgPlayerCamHeight = 30;
    bool dbgEnableDmgZones = false;

    MeshHandle meshPlayerShip;
    MeshHandle meshEyeEn1;

    EntityComponentSystem ecs;

    Room room;
    DamageWorld dmgWorld;
    i32 playerEid = -1;
    i32 weapBulletCount = 0;

    CTransform compTransform[1000];
    CDmgZone compDmgBody[1000];
    i32 compTransformCount = 0;
    i32 compDmgBodyCount = 0;

    bool init();
    void deinit();

    void handlEvent(const SDL_Event& event);
    void update(f64 delta);
    void render();
};
