#pragma once
#include "vector_math.h"
#include "collision.h"

typedef Transform CTransform;
typedef PhysBody CPhysBody;

struct CDmgBody
{
    typedef i32 DamageWorldTeam;
    DamageWorldTeam team;
    Collider collider;
};
