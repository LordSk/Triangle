#pragma once

#include "vector_math.h"
#include <SDL2/SDL_events.h>

struct PlayerShip
{
    Transform tf;
    quat baseRot;
    mat4 mtxModel;

    vec3 vel;
    vec3 mousePosScreen;
    vec3 mousePosWorld;
    f32 angle;

    struct {
        bool8 left;
        bool8 right;
        bool8 up;
        bool8 down;
    } input = {};

    PlayerShip();
    void computeModelMatrix();
    void handleEvent(const SDL_Event& event);
    void update(f64 delta);
    void computeCursorPos(const mat4& invViewProj, f32 camHeight);
};
