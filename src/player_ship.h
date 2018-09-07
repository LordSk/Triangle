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

//@Component
struct CShipWeapon
{
    f32 rateOfFire = 2.0f; // bullets per second
    f32 fireCd = 0.0f;
    f32 bulletSpeed = 60.f;
    i32 dmgTeam = 0;

    //
    MeshHandle hMeshBullet = MESH_HANDLE_INVALID;
    vec4 meshColor = vec4{1, 1, 1, 1};
};

void updatePlayerShipMovement(struct EntityComponentSystem* ecs, CPlayerShipMovement* eltList,
                               const i32 count, const i32* entityId, f64 delta, f64 physLerpAlpha);
void onDeletePlayerShipMovement(struct EntityComponentSystem* ecs, CPlayerShipMovement* eltList,
                                 const i32 count,const i32* entityId, bool8* entDeleteFlag);

void updateShipWeapon(struct EntityComponentSystem* ecs, CShipWeapon* eltList,
                               const i32 count, const i32* entityId, f64 delta, f64 physLerpAlpha);
void onDeleteShipWeapon(struct EntityComponentSystem* ecs, CShipWeapon* eltList,
                                 const i32 count,const i32* entityId, bool8* entDeleteFlag);
