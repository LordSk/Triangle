#include "ecs.h"

i32 EntityComponentSystem::createEntity()
{
    for(i32 i = 0; i < MAX_ENTITIES; ++i) {
        if(entityCompBits[i] == 0) {
            return i;
        }
    }
    assert(0);
    return -1;
}

void EntityComponentSystem::deleteEntity(const i32 eid)
{
    assert(eid >= 0 && eid < MAX_ENTITIES);
    entityDeleteFlag[eid] = true;
}

void EntityComponentSystem::update(f64 delta, f64 physLerpAlpha)
{
#define UC(component) update##component(this, comp_##component.data(), comp_##component.count(),\
    comp_##component._dataEltId.data(), delta, physLerpAlpha)

    UC(PhysBody);
    UC(DmgBody);
    UC(AiBasicEnemy);
    UC(DrawMesh);

#undef UC
}

void EntityComponentSystem::removeFlaggedForDeletion()
{
#define DELETE_FUNC(component) onDelete##component(this, comp_##component.data(), comp_##component.count(),\
    comp_##component._dataEltId.data(), entityDeleteFlag)

    DELETE_FUNC(PhysBody);

#undef DELETE_FUNC

    for(i32 i = 0; i < MAX_ENTITIES; ++i) {
        if(entityDeleteFlag[i]) {
            u64& compBits = entityCompBits[i];

            if(compBits & ComponentBit::Transform) {
                comp_Transform.removeById(i);
            }
            if(compBits & ComponentBit::PhysBody) {
                comp_PhysBody.removeById(i);
            }
            if(compBits & ComponentBit::DmgBody) {
                comp_DmgBody.removeById(i);
            }
            if(compBits & ComponentBit::AiBasicEnemy) {
                comp_AiBasicEnemy.removeById(i);
            }
            if(compBits & ComponentBit::DrawMesh) {
                comp_DrawMesh.removeById(i);
            }
            compBits = 0;
            entityDeleteFlag[i] = false;
        }
    }
}
