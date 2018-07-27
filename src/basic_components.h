#pragma once
#include "vector_math.h"
#include "collision.h"
#include "mesh_load.h"

typedef Transform CTransform;

struct CPhysBody
{
    struct PhysWorld* world = nullptr;
    i32 bodyId = -1;
};

struct CDmgBody
{
    typedef i32 DamageWorldTeam;
    DamageWorldTeam team;
    Collider collider;
};

struct CAiBasicEnemy
{
    f32 changeRightDirCd = 0;
    f32 changeFwdDirCd = 0;
};

struct CDrawMesh
{
    MeshHandle hMesh;
    Transform tf;
    vec4 color;
};

void updatePhysBody(struct EntityComponentSystem* ecs, CPhysBody* eltList, const i32 count, i32* entityId,
                    f64 delta, f64 physLerpAlpha);
void updateDmgBody(struct EntityComponentSystem* ecs, CDmgBody* eltList, const i32 count, i32* entityId,
                   f64 delta, f64 physLerpAlpha);
void updateAiBasicEnemy(struct EntityComponentSystem* ecs, CAiBasicEnemy* eltList, const i32 count,
                        i32* entityId, f64 delta, f64 physLerpAlpha);
void updateDrawMesh(struct EntityComponentSystem* ecs, CDrawMesh* eltList, const i32 count,
                    i32* entityId, f64 delta, f64 physLerpAlpha);

void onDeletePhysBody(struct EntityComponentSystem* ecs, CPhysBody* eltList, const i32 count, i32* entityId,
                      bool8* deleteFlag);
