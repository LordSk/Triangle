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
    bool8 entityDeleteFlag[MAX_ENTITIES] = {0};
    ArraySparse<CTransform> comp_Transform;
    ArraySparse<CPhysBody> comp_PhysBody;
    ArraySparse<CDmgBody> comp_DmgBody;
    ArraySparse<CAiBasicEnemy> comp_AiBasicEnemy;
    ArraySparse<CDrawMesh> comp_DrawMesh;

    i32 createEntity();
    void deleteEntity(const i32 eid);
    void update(f64 delta, f64 physLerpAlpha);
    void removeFlaggedForDeletion();

#define ADD_FUNC(COMP)\
    inline auto& addComp##COMP(const i32 eid) {\
        assert(eid >= 0 && eid < MAX_ENTITIES);\
        entityCompBits[eid] |= ComponentBit::COMP;\
        return comp_##COMP.emplace(eid, {});\
    }

    ADD_FUNC(Transform)
    ADD_FUNC(PhysBody)
    ADD_FUNC(DmgBody)
    ADD_FUNC(AiBasicEnemy)
    ADD_FUNC(DrawMesh)

#undef ADD_FUNC

    // fancy template getter
    template<typename T>
    inline T& getComp(const i32 eid) {
        return _getComp_imp<T>(eid);
    }

    template<typename T>
    inline auto& _getComp_imp(const i32 eid) {
        assert(0);
    }
};

// template specialization for fancy getter
#define GETTER_FUNC(COMP) template<>\
inline auto& EntityComponentSystem::_getComp_imp<C##COMP>(const i32 eid) {\
    assert(eid >= 0 && eid < MAX_ENTITIES);\
    return comp_##COMP[eid];\
}

GETTER_FUNC(Transform)
GETTER_FUNC(PhysBody)
GETTER_FUNC(DmgBody)
GETTER_FUNC(AiBasicEnemy)
GETTER_FUNC(DrawMesh)

#undef GETTER_FUNC
