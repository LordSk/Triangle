#pragma once
#include "vector_math.h"
#include "collision.h"
#include "mesh_load.h"
#include "damage.h"
#include "renderer.h"

//@Component
struct CTransform : Transform
{
};

//@Component(order=0)
struct CPhysBody
{
    i32 bodyId = -1;

    inline void makeCircleBody(const vec2& pos, f32 radius, f32 weight, f32 bounceStr) {
        PhysBody body{};
        Collider collider;
        collider.makeCb(CircleBound{ pos, radius });
        body.pos = pos;
        body.weight = weight;
        body.bounceStrength = bounceStr;
        getPhysWorld().addDynamicBody(collider, body, &bodyId);
    }

    inline PhysBody& getActualBody() {
        return getPhysWorld().bodyDyn[bodyId];
    }
};

// this order thing is not that great, but it works for now
//@Component(order=1)
struct CDmgZone
{
    typedef i32 DamageWorldTeam;
    DamageWorldTeam team;
    Collider collider;
    i32 tag = 0;
    void* userData = nullptr;
    ArraySlice<DamageFrame::IntersectInfo> lastFrameInterList = {};
};

//@Component
struct CEnemyBasicMovement
{

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
    vec2 vel;
};

//@Component
struct CBulletLogic
{
    //
};

void updateBulletLogic(struct EntityComponentSystem* ecs, CBulletLogic* eltList, const i32 count,
                      const i32* entityId, f64 delta, f64 physLerpAlpha);
void onDeleteBulletLogic(struct EntityComponentSystem* ecs, CBulletLogic* eltList, const i32 count,
                        const i32* entityId, bool8* entDeleteFlag);

//@Component
struct CShipInput
{
    bool8 left;
    bool8 right;
    bool8 up;
    bool8 down;
    bool8 fire;
    f32 mouseX;
    f32 mouseY;
};

//@Component
struct CShipControllerHuman
{
    // depends: CShipInput
};

//@Component
struct CShipControllerAi
{
    // depends: CShipInput
    f32 changeRightDirCd = 0;
    f32 changeFwdDirCd = 0;
};

//@Component
struct CHealthCore
{
    f32 healthMax = 100;
    f32 health = 100;
    //
};

//@Component
struct CLightPoint
{
    Transform tf;
    LightPoint light;
};

void updateLightPoint(struct EntityComponentSystem* ecs, CLightPoint* eltList, const i32 count,
                      const i32* entityId, f64 delta, f64 physLerpAlpha);
void onDeleteLightPoint(struct EntityComponentSystem* ecs, CLightPoint* eltList, const i32 count,
                        const i32* entityId, bool8* entDeleteFlag);

void updateHealthCore(struct EntityComponentSystem* ecs, CHealthCore* eltList, const i32 count,
                      const i32* entityId, f64 delta, f64 physLerpAlpha);
void onDeleteHealthCore(struct EntityComponentSystem* ecs, CHealthCore* eltList, const i32 count,
                        const i32* entityId, bool8* entDeleteFlag);

void updateTransform(struct EntityComponentSystem* ecs, CTransform* eltList, const i32 count,const i32* entityId,
                    f64 delta, f64 physLerpAlpha);
void updatePhysBody(struct EntityComponentSystem* ecs, CPhysBody* eltList, const i32 count,const i32* entityId,
                    f64 delta, f64 physLerpAlpha);
void updateDmgZone(struct EntityComponentSystem* ecs, CDmgZone* eltList, const i32 count,const i32* entityId,
                   f64 delta, f64 physLerpAlpha);
void updateEnemyBasicMovement(struct EntityComponentSystem* ecs, CEnemyBasicMovement* eltList,
                               const i32 count, const i32* entityId, f64 delta, f64 physLerpAlpha);
void updateDrawMesh(struct EntityComponentSystem* ecs, CDrawMesh* eltList, const i32 count,
                   const i32* entityId, f64 delta, f64 physLerpAlpha);
void updateBulletMovement(struct EntityComponentSystem* ecs, CBulletMovement* eltList, const i32 count,
                   const i32* entityId, f64 delta, f64 physLerpAlpha);
void updateShipInput(struct EntityComponentSystem* ecs, CShipInput* eltList,
                     const i32 count, const i32* entityId, f64 delta, f64 physLerpAlpha);
void updateShipControllerHuman(struct EntityComponentSystem* ecs, CShipControllerHuman* eltList,
                               const i32 count, const i32* entityId, f64 delta, f64 physLerpAlpha);
void updateShipControllerAi(struct EntityComponentSystem* ecs, CShipControllerAi* eltList,
                               const i32 count, const i32* entityId, f64 delta, f64 physLerpAlpha);

void onDeleteTransform(struct EntityComponentSystem* ecs, CTransform* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag);
void onDeletePhysBody(struct EntityComponentSystem* ecs, CPhysBody* eltList, const i32 count,
                     const i32* entityId, bool8* entDeleteFlag);
void onDeleteDmgZone(struct EntityComponentSystem* ecs, CDmgZone* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag);
void onDeleteEnemyBasicMovement(struct EntityComponentSystem* ecs, CEnemyBasicMovement* eltList,
                                const i32 count, const i32* entityId, bool8* entDeleteFlag);
void onDeleteDrawMesh(struct EntityComponentSystem* ecs, CDrawMesh* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag);
void onDeleteBulletMovement(struct EntityComponentSystem* ecs, CBulletMovement* eltList, const i32 count,
                      const i32* entityId, bool8* entDeleteFlag);
void onDeleteShipInput(struct EntityComponentSystem* ecs, CShipInput* eltList,
                       const i32 count,const i32* entityId, bool8* entDeleteFlag);
void onDeleteShipControllerHuman(struct EntityComponentSystem* ecs, CShipControllerHuman* eltList,
                                 const i32 count,const i32* entityId, bool8* entDeleteFlag);
void onDeleteShipControllerAi(struct EntityComponentSystem* ecs, CShipControllerAi* eltList,
                              const i32 count,const i32* entityId, bool8* entDeleteFlag);
