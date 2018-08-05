#pragma once

#include <SDL2/SDL_events.h>
#include "vector_math.h"
#include "basic_components.h"

//@Component
struct CPlayerShipMovement
{
    f32 accel = 300.0f;
    f32 deccel = 15.0f;
    f32 maxSpeed = 40.0f;
    vec3 curXyPlanePos;
};

void updatePlayerShipMovement(struct EntityComponentSystem* ecs, CPlayerShipMovement* eltList,
                               const i32 count, const i32* entityId, f64 delta, f64 physLerpAlpha);
void onDeletePlayerShipMovement(struct EntityComponentSystem* ecs, CPlayerShipMovement* eltList,
                                 const i32 count,const i32* entityId, bool8* entDeleteFlag);
