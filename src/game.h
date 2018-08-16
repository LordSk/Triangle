#pragma once

#include <SDL2/SDL_events.h>
#include "base.h"
#include "utils.h"
#include "vector_math.h"
#include "collision.h"
#include "player_ship.h"
#include "mesh_load.h"
#include "ecs.h"

struct CameraID
{
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

    void computeCamera(struct Camera* cam);
};

struct CameraFreeFlight
{
    vec3 pos = {0, 0, 10};
    vec3 dir = {1, 0, 0};
    vec3 up = {0, 0, 1};
    quat rot;
    f32 speed = 30;
    f64 pitch = 0;
    f64 yaw = 0;

    struct {
        bool8 forward;
        bool8 backward;
        bool8 left;
        bool8 right;
        bool8 up;
        bool8 down;
    } input = {};

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
    Array<vec4> cubeColors;
    Array<mat4> cubeTfMtx;
    Array<Collider> wallColliders;

    void make(vec3 size_, const i32 cubeSize);
    void addWallsToDamageFrame();
};

struct GameData
{
    f64 time = 0;
    f64 physWorldTimeAcc = 0;
    i32 cameraId = CameraID::PLAYER_VIEW;

    bool mouseCaptured = true;
    CameraFreeFlight camFreeView;

    f32 dbgPlayerCamHeight = 30;
    bool dbgEnableDmgZones = false;

    MeshHandle meshPlayerShip;
    MeshHandle meshEyeEn1;
    MeshHandle meshBullet1;

    EntityComponentSystem ecs;

    Room room;
    i32 playerEid = -1;
    i32 weapBulletCount = 0;

    bool init();
    void deinit();

    void handlEvent(const SDL_Event& event);
    void update(f64 delta);
    void render();

    void componentDoUi(const i32 eid, const u64 compBit);
};
