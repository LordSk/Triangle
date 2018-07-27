#pragma once
#include "utils.h"
#include "basic_components.h"

struct ComponentBit
{
    enum Enum: i32 {
        Null = 0,
        Transform = (1 << 0),
        PhysBody = (1 << 1),
        DmgBody = (1 << 2),
        AiBasicEnemy = (1 << 3),
        DrawMesh = (1 << 4),
    };
};

#define MAX_ENTITIES 2048

struct EntityComponentSystem
{
    u64 entityCompBits[MAX_ENTITIES] = {};
    ArraySparse<CTransform> comp_Transform;
    ArraySparse<CPhysBody> comp_PhysBody;
    ArraySparse<CDmgBody> comp_DmgBody;
    ArraySparse<CAiBasicEnemy> comp_AiBasicEnemy;
    ArraySparse<DrawMesh> comp_DrawMesh;

    i32 createEntity();
    void deleteEntity(const i32 eid);
    void update(f64 delta, f64 physLerpAlpha);

    inline auto& addCompTransform(const i32 eid) {
        assert(eid >= 0 && eid < MAX_ENTITIES);
        entityCompBits[eid] |= ComponentBit::Transform;
        return comp_Transform.emplace(eid, {});
    }

    inline auto& addCompPhysBody(const i32 eid) {
        assert(eid >= 0 && eid < MAX_ENTITIES);
        entityCompBits[eid] |= ComponentBit::PhysBody;
        return comp_PhysBody.emplace(eid, {});
    }

    inline auto& addCompDmgBody(const i32 eid) {
        assert(eid >= 0 && eid < MAX_ENTITIES);
        entityCompBits[eid] |= ComponentBit::DmgBody;
        return comp_DmgBody.emplace(eid, {});
    }

    inline auto& addAiBasicEnemy(const i32 eid) {
        assert(eid >= 0 && eid < MAX_ENTITIES);
        entityCompBits[eid] |= ComponentBit::AiBasicEnemy;
        return comp_AiBasicEnemy.emplace(eid, {});
    }

    inline auto& addDrawMesh(const i32 eid) {
        assert(eid >= 0 && eid < MAX_ENTITIES);
        entityCompBits[eid] |= ComponentBit::DrawMesh;
        return comp_DrawMesh.emplace(eid, {});
    }
};
