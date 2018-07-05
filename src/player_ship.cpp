#include "player_ship.h"
#include <bx/math.h>


PlayerShip::PlayerShip()
{
    pos = {0, 0, 0};
    scale = {0.5, 0.5, 0.5};
    rotation = {0, 0, 0, 1};
    vel = {};
    mousePosScreen = {};
    mousePosWorld = {};

    bx::quatRotateX(rotation, -bx::kPiHalf);
    quat rotZ;
    bx::quatRotateZ(rotZ, -bx::kPiHalf);
    bx::quatMul(rotation, rotation, rotZ);
}

void PlayerShip::computeModelMatrix()
{
    mat4 mtxTrans, mtxRot, mtxScale;
    bx::mtxTranslate(mtxTrans, pos.x, pos.y, pos.z);
    bx::mtxQuat(mtxRot, rotation);
    bx::mtxScale(mtxScale, scale.x, scale.y, scale.z);

    bx::mtxMul(mtxModel, mtxScale, mtxRot);
    bx::mtxMul(mtxModel, mtxModel, mtxTrans);
}

void PlayerShip::handleEvent(const SDL_Event& event)
{
    if(event.type == SDL_KEYDOWN && event.key.repeat == 0) {
        switch(event.key.keysym.sym) {
            case SDLK_UP: input.up = true; break;
            case SDLK_DOWN: input.down = true; break;
            case SDLK_LEFT: input.left = true; break;
            case SDLK_RIGHT: input.right = true; break;
        }
        return;
    }

    if(event.type == SDL_KEYUP) {
        switch(event.key.keysym.sym) {
            case SDLK_UP: input.up = false; break;
            case SDLK_DOWN: input.down = false; break;
            case SDLK_LEFT: input.left = false; break;
            case SDLK_RIGHT: input.right = false; break;
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
}

void PlayerShip::update(f64 delta)
{
    f64 speed = 5;
    vec3 dir = {0, 0, 0};
    dir.x = (input.right - input.left);
    dir.y = (input.up - input.down);

    if(bx::vec3Length(dir) > 0) {
        bx::vec3Norm(dir, dir);
        vel = dir * speed;
    }
    else {
        // apply friction
        vel = vel * (1.0 - bx::clamp(5.0 * delta, 0.0, 1.0));
    }

    pos += vel * delta;
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

    vec3 eye = pos + vec3{0, 0, camHeight};
    vec3 p0 = {0, 0, 0};
    vec3 pn = {0, 0, 1};
    planeLineIntersection(&mousePosWorld, eye, camDir, p0, pn);
}
