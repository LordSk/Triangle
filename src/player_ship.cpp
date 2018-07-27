#include <bx/math.h>
#include "player_ship.h"
#include "collision.h"

void PlayerShip::init()
{
    tf->scale = {0.5, 0.5, 0.5};
    mousePosScreen = {};
    mousePosWorld = {10, 0, 0};
    angle = 0;

    bx::quatRotateX(baseRot, -bx::kPiHalf);
    quat rotZ;
    bx::quatRotateZ(rotZ, -bx::kPiHalf);
    bx::quatMul(baseRot, baseRot, rotZ);
    tf->rot = baseRot;
}

void PlayerShip::handleEvent(const SDL_Event& event)
{
    if(event.type == SDL_KEYDOWN && event.key.repeat == 0) {
        switch(event.key.keysym.sym) {
            case SDLK_z: input.up = true; break;
            case SDLK_s: input.down = true; break;
            case SDLK_q: input.left = true; break;
            case SDLK_d: input.right = true; break;
        }
        return;
    }

    if(event.type == SDL_KEYUP) {
        switch(event.key.keysym.sym) {
            case SDLK_z: input.up = false; break;
            case SDLK_s: input.down = false; break;
            case SDLK_q: input.left = false; break;
            case SDLK_d: input.right = false; break;
        }
        return;
    }

    // TODO: move this
    if(event.type == SDL_MOUSEMOTION) {
        mousePosScreen.x += event.motion.xrel;
        mousePosScreen.y += event.motion.yrel;
        mousePosScreen.x = clamp(mousePosScreen.x, -800.f, 800.f);
        mousePosScreen.y = clamp(mousePosScreen.y, -450.f, 450.f);
        return;
    }

    if(event.type == SDL_MOUSEBUTTONDOWN) {
        switch(event.button.button) {
            case SDL_BUTTON_LEFT: input.fire = true; break;
            case SDL_BUTTON_RIGHT: break;
        }
        return;
    }

    if(event.type == SDL_MOUSEBUTTONUP) {
        switch(event.button.button) {
            case SDL_BUTTON_LEFT: input.fire = false; break;
            case SDL_BUTTON_RIGHT: break;
        }
        return;
    }
}

void PlayerShip::update(f64 delta, f64 physWorldAlpha)
{
    assert(physBody);
    vec2 pos2 = vec2Lerp(physBody->prevPos, physBody->pos, physWorldAlpha);
    vec2 mouseDir = vec2Norm(vec2{ mousePosWorld.x - pos2.x, mousePosWorld.y - pos2.y });
    angle = bx::atan2(-mouseDir.y, mouseDir.x);

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
    f32 accel = 300.0f;
    f32 deccel = 15.0f;

    if(vec2Len(dir) > 0) {
        dir = vec2Norm(dir);
        physBody->vel += dir * accel * delta;
    }
    else {
        // apply friction
        physBody->vel = physBody->vel * (1.0 - bx::clamp(deccel * delta, 0.0, 1.0));
    }

    const f32 maxSpeed = 40.0f;
    if(vec2Len(physBody->vel) > maxSpeed) {
        physBody->vel = vec2Norm(physBody->vel) * maxSpeed;
    }
#endif


    tf->pos.x = pos2.x;
    tf->pos.y = pos2.y;

    bx::quatRotateZ(tf->rot, angle);
    bx::quatMul(tf->rot, baseRot, tf->rot);
}

void PlayerShip::computeCursorPos(const mat4& invViewProj, f32 camHeight)
{
    // TODO: plug view size in
    const i32 WINDOW_WIDTH = 1600;
    const i32 WINDOW_HEIGHT = 900;

    vec4 screenPosNormNear = { mousePosScreen.x / (WINDOW_WIDTH * 0.5f),
                              -mousePosScreen.y / (WINDOW_HEIGHT * 0.5f), 0.0, 1.0 };
    vec4 screenPosNormFar  = { mousePosScreen.x / (WINDOW_WIDTH * 0.5f),
                              -mousePosScreen.y / (WINDOW_HEIGHT * 0.5f), 1.0, 1.0 };

    vec4 worldPosNear, worldPosFar;
    bx::vec4MulMtx(worldPosNear, screenPosNormNear, invViewProj);
    bx::vec4MulMtx(worldPosFar, screenPosNormFar, invViewProj);

    bx::vec3Mul(worldPosNear, worldPosNear, 1.0 / worldPosNear.w);
    bx::vec3Mul(worldPosFar, worldPosFar, 1.0 / worldPosFar.w);
    vec3 worldDir;
    bx::vec3Sub(worldDir, worldPosFar, worldPosNear);
    vec3 camDir;
    bx::vec3Norm(camDir, worldDir);

    vec3 eye = tf->pos + vec3{0, 0, camHeight};
    vec3 p0 = {0, 0, 0};
    vec3 pn = {0, 0, 1};
    planeLineIntersection(&mousePosWorld, eye, camDir, p0, pn);
}
