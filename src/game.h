#pragma once

#include "base.h"
#include "utils.h"
#include "vector_math.h"
#include "collision.h"
#include "player_ship.h"
#include "mesh_load.h"
#include <SDL2/SDL_events.h>

struct Room
{
    Transform tfRoom;
    vec3 size;
    Array<Transform> cubeTransforms;
    Array<mat4> cubeTfMtx;
    PhysWorld physWorld;

    void make(vec3 size_, const i32 cubeSize);

    void dbgDrawPhysWorld();
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

    void compute(mat4* lookAt) {
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

    void handleEvent(const SDL_Event& event) {
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

    void applyInput(f64 delta) {
        vec3 right;
        bx::vec3Cross(right, dir, up);
        bx::vec3Norm(right, right);

        pos += dir * speed * delta * (input.forward - input.backward);
        pos += up * speed * delta * (input.up - input.down);
        pos += right * speed * delta * (input.right - input.left);
    }
};


struct GameData
{
    MeshHandle meshPlayerShip;
    MeshHandle meshEyeEn1;

    Room room;
    PlayerShip playerShip;
    PhysBody* playerBody;

    f64 time = 0;
    f64 physWorldTimeAcc = 0;
    i32 cameraId = CameraID::PLAYER_VIEW;

    bool mouseCaptured = true;
    CameraFreeFlight camFree;

    f32 dbgPlayerCamHeight = 30;

    bool init();
    void deinit();

    void handlEvent(const SDL_Event& event);
    void update(f64 delta);
    void render();
};