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
    u64& compBits = entityCompBits[eid];

    if(compBits & ComponentBit::Transform) {
        comp_Transform.removeById(eid);
    }
    if(compBits & ComponentBit::PhysBody) {
        comp_PhysBody.removeById(eid);
    }
    if(compBits & ComponentBit::DmgBody) {
        comp_DmgBody.removeById(eid);
    }
    if(compBits & ComponentBit::AiBasicEnemy) {
        comp_AiBasicEnemy.removeById(eid);
    }
    if(compBits & ComponentBit::DrawMesh) {
        comp_DrawMesh.removeById(eid);
    }
    compBits = 0;
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
