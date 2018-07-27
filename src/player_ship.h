#pragma once

#include <SDL2/SDL_events.h>
#include "vector_math.h"
#include "basic_components.h"

struct PlayerShip
{
    CTransform* tf;
    PhysWorld* physWorld;
    i32 physBodyId;
    CDmgBody* dmgBody;
    quat baseRot;

    vec3 mousePosScreen;
    vec3 mousePosWorld;
    f32 angle;

    struct {
        bool8 left;
        bool8 right;
        bool8 up;
        bool8 down;
        bool8 fire;
    } input = {};

    void init();
    void handleEvent(const SDL_Event& event);
    void update(f64 delta, f64 physWorldAlpha);
    void computeCursorPos(const mat4& invViewProj, f32 camHeight);
};

struct WeaponBullet
{
    CTransform* tf;
    CDmgBody* dmgBody;
    vec2 pos;
    vec2 vel;
};
