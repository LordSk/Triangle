#pragma once
#include "vector_math.h"
#include "collision.h"
#include "mesh_load.h"

//@Component
struct CTransform : Transform
{
};

//@Component
struct CPhysBody
{
    struct PhysWorld* world = nullptr;
    i32 bodyId = -1;
};

//@Component
struct CDmgZone
{
    typedef i32 DamageWorldTeam;
    DamageWorldTeam team;
    Collider collider;
};

//@Component
struct CAiBasicEnemy
{
    f32 changeRightDirCd = 0;
    f32 changeFwdDirCd = 0;
};

//@Component
struct CDrawMesh
{
    MeshHandle hMesh;
    Transform tf;
    vec4 color;
};

//@Component
struct CBulletMovement
{
    vec2 pos;
    vec2 vel;
};

//@Component
struct CInputShipController
{
    bool8 left;
    bool8 right;
    bool8 up;
    bool8 down;
    bool8 fire;
    f32 mouseX;
    f32 mouseY;
};

void updateTransform(struct EntityComponentSystem* ecs, CTransform* eltList, const i32 count,const i32* entityId,
                    f64 delta, f64 physLerpAlpha);
void updatePhysBody(struct EntityComponentSystem* ecs, CPhysBody* eltList, const i32 count,const i32* entityId,
                    f64 delta, f64 physLerpAlpha);
void updateDmgZone(struct EntityComponentSystem* ecs, CDmgZone* eltList, const i32 count,const i32* entityId,
                   f64 delta, f64 physLerpAlpha);
void updateAiBasicEnemy(struct EntityComponentSystem* ecs, CAiBasicEnemy* eltList, const i32 count,
                       const i32* entityId, f64 delta, f64 physLerpAlpha);
void updateDrawMesh(struct EntityComponentSystem* ecs, CDrawMesh* eltList, const i32 count,
                   const i32* entityId, f64 delta, f64 physLerpAlpha);
void updateBulletMovement(struct EntityComponentSystem* ecs, CBulletMovement* eltList, const i32 count,
                   const i32* entityId, f64 delta, f64 physLerpAlpha);
void updateInputShipController(struct EntityComponentSystem* ecs, CInputShipController* eltList,
                               const i32 count, const i32* entityId, f64 delta, f64 physLerpAlpha);

void onDeleteTransform(struct EntityComponentSystem* ecs, CTransform* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag);
void onDeletePhysBody(struct EntityComponentSystem* ecs, CPhysBody* eltList, const i32 count,
                     const i32* entityId, bool8* entDeleteFlag);
void onDeleteDmgZone(struct EntityComponentSystem* ecs, CDmgZone* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag);
void onDeleteAiBasicEnemy(struct EntityComponentSystem* ecs, CAiBasicEnemy* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag);
void onDeleteDrawMesh(struct EntityComponentSystem* ecs, CDrawMesh* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag);
void onDeleteBulletMovement(struct EntityComponentSystem* ecs, CBulletMovement* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag);
void onDeleteInputShipController(struct EntityComponentSystem* ecs, CInputShipController* eltList,
                            const i32 count,const i32* entityId, bool8* entDeleteFlag);
