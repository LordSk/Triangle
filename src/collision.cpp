#include "collision.h"
#include "dbg_draw.h"
#include <imgui/imgui.h>

// sepearatig axis theorem
bool obbIntersectObb(const OrientedBoundingBox& obbA, const OrientedBoundingBox& obbB, CollisionInfo* out)
{
    const f32 cosA = cosf(obbA.angle);
    const f32 sinA = sinf(obbA.angle);
    const f32 cosB = cosf(obbB.angle);
    const f32 sinB = sinf(obbB.angle);

    const vec2 pmaxA = vec2Rotate(obbA.size, cosA, sinA);
    const vec2 pmaxB = vec2Rotate(obbB.size, cosB, sinB);

    // A
    const vec2 centerA = obbA.origin + pmaxA * 0.5f;
    const vec2 xA = vec2{ cosA, -sinA };
    const vec2 yA = vec2{ sinA, cosA };
    const f32 halfWidthA = obbA.size.x * 0.5f;
    const f32 halfHeightA = obbA.size.y * 0.5f;

    // B
    const vec2 centerB = obbB.origin + pmaxB * 0.5f;
    const vec2 xB = vec2{ cosB, -sinB };
    const vec2 yB = vec2{ sinB, cosB };
    const f32 halfWidthB = obbB.size.x * 0.5f;
    const f32 halfHeightB = obbB.size.y * 0.5f;

    const vec2 T = centerB - centerA;

    //| T • Ax | > WA + | ( WB*Bx ) • Ax | + |( HB*By ) • Ax |
    const f32 dxA = vec2Dot(T, xA);
    const f32 cmpxA = halfWidthA + abs(vec2Dot(xB * halfWidthB, xA)) + abs(vec2Dot(yB * halfHeightB, xA));
    if(abs(dxA) > cmpxA) {
        return false;
    }

    //| T • Ay | > HA + | ( WB*Bx ) • Ay | + |( HB*By ) • Ay |
    const f32 dyA = vec2Dot(T, yA);
    const f32 cmpyA = halfHeightA + abs(vec2Dot(xB * halfWidthB, yA)) + abs(vec2Dot(yB * halfHeightB, yA));
    if(abs(dyA) > cmpyA) {
        return false;
    }

    //| T • Bx | > | ( WA* Ax ) • Bx | + | ( HA*Ay ) • Bx | + WB
    const f32 dxB = vec2Dot(T, xB);
    const f32 cmpxB = halfWidthB + abs(vec2Dot(xA * halfWidthA, xB)) + abs(vec2Dot(yA * halfHeightA, xB));
    if(abs(dxB) > cmpxB) {
        return false;
    }

    //| T • By | > | ( WA* Ax ) • By | + | ( HA*Ay ) • By | + HB
    const f32 dyB = vec2Dot(T, yB);
    const f32 cmpyB = halfHeightB + abs(vec2Dot(xA * halfWidthA, yB)) + abs(vec2Dot(yA * halfHeightA, yB));
    if(abs(dyB) > cmpyB) {
        return false;
    }

    const vec2 pvxA = xA * ((abs(dxA) - cmpxA) * signf(dxA));
    const f32 pvxAlen = vec2Len(pvxA);
    vec2 pv = pvxA;
    f32 pvLen = pvxAlen;

    const vec2 pvyA = yA * ((abs(dyA) - cmpyA) * signf(dyA));
    const f32 pvyAlen = vec2Len(pvyA);
    if(pvyAlen < pvLen) {
        pv = pvyA;
        pvLen = pvyAlen;
    }

    const vec2 pvxB = xB * ((abs(dxB) - cmpxB) * signf(dxB));
    const f32 pvxBlen = vec2Len(pvxB);
    if(pvxBlen < pvLen) {
        pv = pvxB;
        pvLen = pvxBlen;
    }

    const vec2 pvyB = yB * ((abs(dyB) - cmpyB) * signf(dyB));
    const f32 pvyBlen = vec2Len(pvyB);
    if(pvyBlen < pvLen) {
        pv = pvyB;
        pvLen = pvyBlen;
    }

    out->penVec = pv;

    return true;
}

bool obbIntersectCb(const OrientedBoundingBox& obbA, const CircleBound& cbB, CollisionInfo* out)
{
    const f32 obbA_angle = obbA.angle;
    const f32 cosA = cosf(obbA_angle);
    const f32 sinA = sinf(obbA_angle);

    const vec2 pmaxA = vec2Rotate(obbA.size, cosA, sinA);

    // A
    const vec2 xA = vec2{ cosA, -sinA };
    const vec2 yA = vec2{ sinA, cosA };
    const vec2 centerA = obbA.origin + pmaxA * 0.5f;
    const f32 halfWidthA = obbA.size.x * 0.5f;
    const f32 halfHeightA = obbA.size.y * 0.5f;
    const f32 halfDiagA = vec2Len(obbA.size) * 0.5;

    const vec2 vecs[3] = {
        vec2{ obbA.size.x, 0 },
        vec2{ 0, obbA.size.y },
        vec2{ obbA.size.x, obbA.size.y }
    };

    const vec2 rotatedVecs[3] = {
        vec2Rotate(vecs[0], cosA, sinA),
        vec2Rotate(vecs[1], cosA, sinA),
        vec2Rotate(vecs[2], cosA, sinA)
    };

    const vec2 orgnA = obbA.origin;
    const vec2 points[4] = {
        orgnA,
        orgnA + rotatedVecs[0],
        orgnA + rotatedVecs[1],
        orgnA + rotatedVecs[2]
    };

    const vec2 centerB = cbB.center;
    const vec2 T = centerB - centerA;
    f32 closestDist = vec2Len(centerB - points[0]);
    vec2 closestPoint = points[0];

    for(i32 i = 1; i < 4; i++) {
        f32 dist = vec2Len(centerB - points[i]);
        if(dist < closestDist) {
            closestDist = dist;
            closestPoint = points[i];
        }
    }

    // | T • Ax | > WA + | ( WB*Bx ) • Ax | + |( HB*By ) • Ax |
    const f32 dxA = vec2Dot(T, xA);
    const f32 cmpxA = halfWidthA + cbB.radius;
    if(abs(dxA) > cmpxA) {
        return false;
    }

    // | T • Ay | > HA + | ( WB*Bx ) • Ay | + |( HB*By ) • Ay |
    const f32 dyA = vec2Dot(T, yA);
    const f32 cmpyA = halfHeightA + cbB.radius;
    if(abs(dyA) > cmpyA) {
        return false;
    }

    const vec2 uA = vec2Norm(closestPoint - centerA);
    const f32 duA = vec2Dot(T, uA);
    const f32 cmpuA = halfDiagA + cbB.radius;
    if(abs(duA) > cmpuA) {
        return false;
    }

    const vec2 pvxA = xA * ((abs(dxA) - cmpxA) * signf(dxA));
    const f32 pvxAlen = vec2Len(pvxA);
    vec2 pv = pvxA;
    f32 pvLen = pvxAlen;

    const vec2 pvyA = yA * ((abs(dyA) - cmpyA) * signf(dyA));
    const f32 pvyAlen = vec2Len(pvyA);
    if(pvyAlen < pvLen) {
        pv = pvyA;
        pvLen = pvyAlen;
    }

    const vec2 pvuA = uA * ((abs(duA) - cmpuA) * signf(duA));
    const f32 pvuAlen = vec2Len(pvuA);
    if(pvuAlen < pvLen) {
        pv = pvuA;
        pvLen = pvuAlen;
    }

    out->penVec = pv;

    return true;
}

bool cbIntersectCb(const CircleBound& cbA, const CircleBound& cbB, CollisionInfo* out)
{
    const vec2 orgnA = cbA.center;
    const vec2 orgnB = cbB.center;

    f32 l = vec2Len(orgnB - orgnA);
    if(l < (cbA.radius + cbB.radius)) {
        out->penVec = vec2Norm(orgnB - orgnA) * (cbA.radius + cbB.radius - l);
        return true;
    }
    return false;
}

bool colliderIntersect(const Collider& col1, const Collider& col2, CollisionInfo* out)
{
    const Collider::Enum type1 = col1.type;
    const Collider::Enum type2 = col2.type;

    if(type1 == Collider::OBB && type2 == Collider::OBB) {
        return obbIntersectObb(col1.obb, col2.obb, out);
    }
    if(type1 == Collider::OBB && type2 == Collider::CIRCLE) {
        return obbIntersectCb(col1.obb, col2.cb, out);
    }
    if(type1 == Collider::CIRCLE && type2 == Collider::CIRCLE) {
        return cbIntersectCb(col1.cb, col2.cb, out);
    }
    if(type1 == Collider::CIRCLE && type2 == Collider::OBB) {
        return obbIntersectCb(col2.obb, col1.cb, out);
    }
    assert(0);
    return false;
}

void obbDbgDraw(const OrientedBoundingBox& obb, vec4 color)
{
    vec3 pmax = vec2ToVec3(vec2Rotate(obb.size, obb.angle));
    vec3 origin3 = vec2ToVec3(obb.origin);
    vec3 centerA = origin3 + pmax * 0.5f;

    const f32 cosA = bx::cos(obb.angle);
    const f32 sinA = bx::sin(obb.angle);
    vec3 xA = vec3{ cosA * obb.size.x * 0.25f, -sinA * obb.size.x * 0.25f, 0 };
    vec3 yA = vec3{ sinA * obb.size.y * 0.25f, cosA * obb.size.y * 0.25f, 0 };

    dbgDrawLine(origin3, origin3 + xA, vec4{0, 0, 1, 1});
    dbgDrawLine(origin3, origin3 + yA, vec4{0, 1, 0, 1});

    Transform tfObb;

    tfObb = {};
    tfObb.pos = centerA;
    tfObb.scale = {0.2f, 0.2f, 0.2f};
    dbgDrawRect(tfObb, {1, 1, 1, 1});

    tfObb = {};
    tfObb.pos = origin3;
    tfObb.scale = vec3{ obb.size.x, obb.size.y, 2 };
    bx::quatRotateZ(tfObb.rot, obb.angle);
    dbgDrawRect(tfObb, color);
}

void cbDbgDraw(const CircleBound& cb, vec4 color)
{
    vec3 center3 = vec2ToVec3(cb.center);
    dbgDrawLine(center3, center3 + vec3{cb.radius, 0, 0}, vec4{0, 0, 1, 1});
    dbgDrawLine(center3, center3 + vec3{0, cb.radius, 0}, vec4{0, 1, 0, 1});
    dbgDrawSphere(center3, cb.radius, color);
}

void colliderDbgDraw(const Collider& col1, vec4 color)
{
    const Collider::Enum type1 = col1.type;
    if(type1 == Collider::OBB) {
        return obbDbgDraw(col1.obb, color);
    }
    else if(type1 == Collider::CIRCLE) {
        return cbDbgDraw(col1.cb, color);
    }
    assert(0);
}

PhysWorld::PhysWorld()
{
    colStatic.reserve(512);
}

void PhysWorld::clear()
{
    colStatic.clear();
    colDynamic.clear();
    bodyDyn.clear();
}

Collider* PhysWorld::addStaticCollider(Collider col)
{
    return &(colStatic.push(col));
}

PhysBody* PhysWorld::addDynamicBody(Collider col, PhysBody body, i32* bid)
{
    colDynamic.push(col);
    return &(bodyDyn.push(body, bid));
}

void PhysWorld::removeBodyById(i32 bid)
{
    colDynamic.removeById(bid);
    bodyDyn.removeById(bid);
}

void PhysWorld::update(f64 delta, const i32 stepCount)
{
    const i32 statCount = colStatic.count();
    Collider* statBodies = colStatic.data();
    const i32 dynColCount = colDynamic.count();
    Collider* dynColliders = colDynamic.data();
    const i32 dynCount = bodyDyn.count();
    PhysBody* dynBodies = bodyDyn.data();

    struct CollisionEvent {
        PhysBody* body1;
        PhysBody* body2;
        vec2 pv;
    };

    assert(dynCount <= 2048);
    CollisionEvent ceList[2048];
    i32 ceListCount = 0;

    for(i32 db = 0; db < dynCount; db++) {
        PhysBody& body = dynBodies[db];
        body.prevPos = body.pos;
    }

    for(i32 step = 0; step < stepCount; step++) {
        bool8 collided[2048] = {0};

        for(i32 db = 0; db < dynCount; db++) {
            PhysBody& body = dynBodies[db];
            Collider& col1 = dynColliders[db];
            body.pos += body.vel * (delta * (1.0 / stepCount));
            col1.setPos(body.pos);
        }

        for(i32 db = 0; db < dynCount; db++) {
            if(collided[db]) continue;

            PhysBody* pBody = &dynBodies[db];
            Collider& col1 = dynColliders[db];

            bool collidedWithStatic = false;
            for(i32 sb = 0; sb < statCount; sb++) {
                CollisionInfo coliInfo;
                if(colliderIntersect(col1, statBodies[sb], &coliInfo) &&
                   vec2Len(coliInfo.penVec) > 0.0001f) {
                    CollisionEvent ce;
                    ce.pv = coliInfo.penVec;
                    ce.body1 = nullptr;
                    ce.body2 = pBody;
                    assert(ceListCount < arr_count(ceList));
                    ceList[ceListCount++] = ce;
                    collidedWithStatic = true;
                }
            }

            if(collidedWithStatic) {
                collided[db] = true;
                continue;
            }

            for(i32 db2 = 0; db2 < dynCount; db2++) {
                if(collided[db2]) continue;

                PhysBody* pBody2 = &dynBodies[db2];
                Collider& col2 = dynColliders[db2];

                // TODO: implement weight
                CollisionInfo coliInfo;
                if(colliderIntersect(col1, col2, &coliInfo) &&
                   vec2Len(coliInfo.penVec) > 0.0001f) {
                    CollisionEvent ce;
                    ce.pv = coliInfo.penVec;
                    ce.body1 = pBody;
                    ce.body2 = pBody2;
                    assert(ceListCount < arr_count(ceList));
                    ceList[ceListCount++] = ce;
                    collided[db] = true;
                    collided[db2] = true;
                    break;
                }
            }
        }

        const i32 ceCount = ceListCount;
        for(i32 c = 0; c < ceCount; c++) {
            CollisionEvent& ce = ceList[c];

            if(!ce.body1) {
                const vec2 pv = ce.pv;
                vec2& body_pos = ce.body2->pos;
                vec2& body_vel = ce.body2->vel;
                const f32 body_bounceStr = ce.body2->bounceStrength;

                const f32 d = vec2Dot(body_vel, pv);
                vec2 rem = pv * (d / vec2Dot(pv, pv));
                body_vel -= rem + rem * body_bounceStr;
                body_pos -= pv * 1.0001f;
            }
            else {
                const vec2 pv = ce.pv;
                vec2& body_pos = ce.body1->pos;
                vec2& body_vel = ce.body1->vel;
                const f32 body_bounceStr = ce.body1->bounceStrength;
                vec2& body2_pos = ce.body2->pos;
                vec2& body2_vel = ce.body2->vel;
                const f32 body2_bounceStr = ce.body2->bounceStrength;

                const f32 d = vec2Dot(body_vel, pv);
                const f32 d2 = vec2Dot(body2_vel, pv);

                const vec2 rem = pv * (d / vec2Dot(pv, pv));
                const vec2 rem2 = pv * (d2 / vec2Dot(pv, pv));

                body_vel -= rem + rem * body_bounceStr;
                body_pos -= pv * 0.5001f;
                body2_vel -= rem2 + rem2 * body2_bounceStr;
                body2_pos += pv * 0.5001f;
            }
        }

        ceListCount = 0;
    }
}

void PhysWorld::dbgDraw(vec3 offset)
{
    const i32 statCount = colStatic.count();
    Collider* statColliders = colStatic.data();
    const i32 dynCount = colDynamic.count();
    Collider* dynColliders = colDynamic.data();

    for(i32 sb = 0; sb < statCount; sb++) {
        Collider col = statColliders[sb];
        col.setPos(col.getPos() + vec3ToVec2(offset));
        colliderDbgDraw(col, vec4{0.0f, 200.f/255.f, 1.0, 0.5f});
    }

    for(i32 db = 0; db < dynCount; db++) {
        Collider col = dynColliders[db];
        col.setPos(col.getPos() + vec3ToVec2(offset));
        colliderDbgDraw(col, vec4{1.0f, 200.f/255.f, 0, 0.5f});
    }
}
