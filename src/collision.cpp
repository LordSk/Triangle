#include "collision.h"
#include "dbg_draw.h"
#include <imgui/imgui.h>

// sepearatig axis theorem
bool obbIntersectObb(const OrientedBoundingBox& obbA, const OrientedBoundingBox& obbB, CollisionInfo* out)
{
    // TODO: do 3D (maybe)

    const f32 cosA = bx::cos(obbA.angle);
    const f32 sinA = bx::sin(obbA.angle);
    const f32 cosB = bx::cos(obbB.angle);
    const f32 sinB = bx::sin(obbB.angle);

    quat qA;
    bx::quatRotateZ(qA, obbA.angle);
    vec3 pmaxA;
    bx::vec3MulQuat(pmaxA, obbA.size, qA);

    quat qB;
    bx::quatRotateZ(qB, obbB.angle);
    vec3 pmaxB;
    bx::vec3MulQuat(pmaxB, obbB.size, qB);

    // A
    vec2 centerA = vec2{ obbA.origin.x, obbA.origin.y } + vec2{ pmaxA.x, pmaxA.y } * 0.5f;
    vec2 xA = vec2{ cosA, -sinA };
    vec2 yA = vec2{ sinA, cosA };
    f32 halfWidthA = obbA.size.x * 0.5f;
    f32 halfHeightA = obbA.size.y * 0.5f;

    // B
    vec2 centerB = vec2{ obbB.origin.x, obbB.origin.y } + vec2{ pmaxB.x, pmaxB.y } * 0.5f;
    vec2 xB = vec2{ cosB, -sinB };
    vec2 yB = vec2{ sinB, cosB };
    f32 halfWidthB = obbB.size.x * 0.5f;
    f32 halfHeightB = obbB.size.y * 0.5f;

    vec2 T = centerB - centerA;

    //| T • Ax | > WA + | ( WB*Bx ) • Ax | + |( HB*By ) • Ax |
    f32 dxA = vec2Dot(T, xA);
    f32 cmp = halfWidthA + bx::abs(vec2Dot(xB * halfWidthB, xA)) + bx::abs(vec2Dot(yB * halfHeightB, xA));
    if(bx::abs(dxA) > cmp) {
        return false;
    }

    const vec2 pvxA = xA * ((bx::abs(dxA) - cmp) * bx::sign(dxA));
    const f32 pvxAlen = vec2Len(pvxA);
    vec2 pv = pvxA;
    f32 pvLen = pvxAlen;

    //| T • Ay | > HA + | ( WB*Bx ) • Ay | + |( HB*By ) • Ay |
    f32 dyA = vec2Dot(T, yA);
    cmp = halfHeightA + bx::abs(vec2Dot(xB * halfWidthB, yA)) + bx::abs(vec2Dot(yB * halfHeightB, yA));
    if(bx::abs(dyA) > cmp) {
        return false;
    }

    const vec2 pvyA = yA * ((bx::abs(dyA) - cmp) * bx::sign(dyA));
    const f32 pvyAlen = vec2Len(pvyA);
    if(pvyAlen < pvLen) {
        pv = pvyA;
        pvLen = pvyAlen;
    }

    //| T • Bx | > | ( WA* Ax ) • Bx | + | ( HA*Ay ) • Bx | + WB
    f32 dxB = vec2Dot(T, xB);
    cmp = halfWidthB + bx::abs(vec2Dot(xA * halfWidthA, xB)) + bx::abs(vec2Dot(yA * halfHeightA, xB));
    if(bx::abs(dxB) > cmp) {
        return false;
    }

    const vec2 pvxB = xB * ((bx::abs(dxB) - cmp) * bx::sign(dxB));
    const f32 pvxBlen = vec2Len(pvxB);
    if(pvxBlen < pvLen) {
        pv = pvxB;
        pvLen = pvxBlen;
    }

    //| T • By | > | ( WA* Ax ) • By | + | ( HA*Ay ) • By | + HB
    f32 dyB = vec2Dot(T, yB);
    cmp = halfHeightB + bx::abs(vec2Dot(xA * halfWidthA, yB)) + bx::abs(vec2Dot(yA * halfHeightA, yB));
    if(bx::abs(dyB) > cmp) {
        return false;
    }

    const vec2 pvyB = yB * ((bx::abs(dyB) - cmp) * bx::sign(dyB));
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
    const f32 cosA = bx::cos(obbA.angle);
    const f32 sinA = bx::sin(obbA.angle);

    quat qA;
    bx::quatRotateZ(qA, obbA.angle);
    vec3 pmaxA;
    bx::vec3MulQuat(pmaxA, obbA.size, qA);

    // A
    const vec2 xA = vec2{ cosA, -sinA };
    const vec2 yA = vec2{ sinA, cosA };
    const vec2 centerA = vec2{ obbA.origin.x, obbA.origin.y } + vec2{ pmaxA.x, pmaxA.y } * 0.5f;
    const f32 halfWidthA = obbA.size.x * 0.5f;
    const f32 halfHeightA = obbA.size.y * 0.5f;
    const f32 halfDiagA = vec2Len(vec2{ obbA.size.x, obbA.size.y }) * 0.5;

    vec3 vecs[3];
    vecs[0] = { obbA.size.x, 0, 0 };
    vecs[1] = { 0, obbA.size.y, 0 };
    vecs[2] = { obbA.size.x, obbA.size.y, 0 };
    vec3 rotatedVecs[3];
    bx::vec3MulQuat(rotatedVecs[0], vecs[0], qA);
    bx::vec3MulQuat(rotatedVecs[1], vecs[1], qA);
    bx::vec3MulQuat(rotatedVecs[2], vecs[2], qA);

    const vec2 orgnA = { obbA.origin.x, obbA.origin.y };
    vec2 points[4];
    points[0] = orgnA;
    points[1] = orgnA + vec2{ rotatedVecs[0].x, rotatedVecs[0].y };
    points[2] = orgnA + vec2{ rotatedVecs[1].x, rotatedVecs[1].y };
    points[3] = orgnA + vec2{ rotatedVecs[2].x, rotatedVecs[2].y };

    const vec2 centerB = vec2{ cbB.center.x, cbB.center.y };
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
    f32 dxA = vec2Dot(T, xA);
    f32 cmp = halfWidthA + cbB.radius;
    if(bx::abs(dxA) > cmp) {
        return false;
    }

    const vec2 pvxA = xA * ((bx::abs(dxA) - cmp) * bx::sign(dxA));
    const f32 pvxAlen = vec2Len(pvxA);
    vec2 pv = pvxA;
    f32 pvLen = pvxAlen;

    // | T • Ay | > HA + | ( WB*Bx ) • Ay | + |( HB*By ) • Ay |
    f32 dyA = vec2Dot(T, yA);
    cmp = halfHeightA + cbB.radius;
    if(bx::abs(dyA) > cmp) {
        return false;
    }

    const vec2 pvyA = yA * ((bx::abs(dyA) - cmp) * bx::sign(dyA));
    const f32 pvyAlen = vec2Len(pvyA);
    if(pvyAlen < pvLen) {
        pv = pvyA;
        pvLen = pvyAlen;
    }

    const vec2 uA = vec2Norm(closestPoint - centerA);
    f32 duA = vec2Dot(T, uA);
    cmp = halfDiagA + cbB.radius;
    if(bx::abs(duA) > cmp) {
        return false;
    }

    const vec2 pvuA = uA * ((bx::abs(duA) - cmp) * bx::sign(duA));
    const f32 pvuAlen = vec2Len(pvuA);
    if(pvuAlen < pvLen) {
        pv = pvuA;
        pvLen = pvuAlen;
    }

    out->penVec = pv;

    /*
    ImGui::Begin("intersect");
    ImGui::Text("dxA: %.5f", dxA);
    ImGui::TextColored(ImVec4(0, 0, 1, 1), "pvxA: { %.5f, %.5f }", pvxA.x, pvxA.y);
    ImGui::Text("dyA: %.5f", dyA);
    ImGui::Text("duA: %.5f", duA);
    ImGui::End();

    dbgDrawLine(vec2ToVec3(centerA), vec2ToVec3(centerB), vec4{0.5, 0, 1, 1});
    dbgDrawLine(obbA.origin, obbA.origin + vec2ToVec3(pvxA), vec4{0, 0, 1, 1});
    dbgDrawLine(obbA.origin, obbA.origin + vec2ToVec3(pvyA), vec4{0, 1, 0, 1});
    dbgDrawLine(obbA.origin, obbA.origin + vec2ToVec3(pvuA), vec4{1, 0, 0, 1});
    */

    return true;
}

bool cbIntersectCb(const CircleBound& cbA, const CircleBound& cbB, CollisionInfo* out)
{
    const vec2 orgnA = { cbA.center.x, cbA.center.y };
    const vec2 orgnB = { cbB.center.x, cbB.center.y };

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
    quat q;
    bx::quatRotateZ(q, obb.angle);
    vec3 pmax;
    bx::vec3MulQuat(pmax, obb.size, q);
    vec3 centerA = obb.origin + pmax * 0.5f;

    const f32 cosA = bx::cos(obb.angle);
    const f32 sinA = bx::sin(obb.angle);
    vec3 xA = vec3{ cosA * obb.size.x * 0.25f, -sinA * obb.size.x * 0.25f, 0 };
    vec3 yA = vec3{ sinA * obb.size.y * 0.25f, cosA * obb.size.y * 0.25f, 0 };

    dbgDrawLine(obb.origin, obb.origin + xA, vec4{0, 0, 1, 1});
    dbgDrawLine(obb.origin, obb.origin + yA, vec4{0, 1, 0, 1});

    Transform tfObb;

    tfObb = {};
    tfObb.pos = centerA;
    tfObb.scale = {0.2f, 0.2f, 0.2f};
    dbgDrawRect(tfObb, {1, 1, 1, 1});

    tfObb = {};
    tfObb.pos = obb.origin;
    tfObb.scale = obb.size;
    bx::quatRotateZ(tfObb.rot, obb.angle);
    dbgDrawRect(tfObb, color);
}

void cbDbgDraw(const CircleBound& cb, vec4 color)
{
    dbgDrawLine(cb.center, cb.center + vec3{cb.radius, 0, 0}, vec4{0, 0, 1, 1});
    dbgDrawLine(cb.center, cb.center + vec3{0, cb.radius, 0}, vec4{0, 1, 0, 1});
    dbgDrawSphere(cb.center, cb.radius, color);
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
    bodies.reserve(512);
}

void PhysWorld::clear()
{
    colStatic.clear();
    bodies.clear();
}

Collider* PhysWorld::addStaticCollider(Collider col)
{
    return &(colStatic.push(col));
}

PhysBody* PhysWorld::addDynamicBody(PhysBody body)
{
    return &(bodies.push(body));
}

void PhysWorld::update(f64 delta, const i32 stepCount)
{
    const i32 statCount = colStatic.count();
    Collider* statBodies = colStatic.data();
    const i32 dynCount = bodies.count();
    PhysBody* dynBodies = bodies.data();

    for(i32 db = 0; db < dynCount; db++) {
        PhysBody& body = dynBodies[db];
        body.prevPos = body.pos;
    }

    for(i32 step = 0; step < stepCount; step++) {
        for(i32 db = 0; db < dynCount; db++) {
            PhysBody& body = dynBodies[db];
            body.pos += body.vel * (delta * (1.0 / stepCount));
            body.col.setPos(body.pos);

            for(i32 sb = 0; sb < statCount; sb++) {
                CollisionInfo coliInfo;
                if(colliderIntersect(body.col, statBodies[sb], &coliInfo) &&
                   vec2Len(coliInfo.penVec) > 0.0001f) {
                    const vec3 pv = vec2ToVec3(coliInfo.penVec);
                    const f32 d = bx::vec3Dot(body.vel, pv);
                    vec3 rem = pv * (d / bx::vec3Dot(pv, pv));
                    body.vel -= rem + rem * body.bounceStrength;
                    body.pos -= pv * 1.0001f;
                    body.col.setPos(body.pos);
                };
            }

            for(i32 db2 = 0; db2 < dynCount; db2++) {
                PhysBody& body2 = dynBodies[db2];

                // TODO: implement weight
                CollisionInfo coliInfo;
                if(colliderIntersect(body.col, body2.col, &coliInfo) &&
                   vec2Len(coliInfo.penVec) > 0.0001f) {
                    const vec3 pv = vec2ToVec3(coliInfo.penVec);
                    const f32 d = bx::vec3Dot(body.vel, pv);
                    const f32 d2 = bx::vec3Dot(body2.vel, pv);
                    vec3 rem = pv * (d / bx::vec3Dot(pv, pv));
                    vec3 rem2 = pv * (d2 / bx::vec3Dot(pv, pv));
                    body.vel -= rem + rem * body.bounceStrength;
                    body2.vel -= rem2 + rem2 * body.bounceStrength;
                    body.pos -= pv * 0.5001f;
                    body2.pos += pv * 0.5001f;
                    body.col.setPos(body.pos);
                    body2.col.setPos(body2.pos);
                };
            }
        }
    }
}

void PhysWorld::dbgDraw(vec3 offset)
{
    const i32 statCount = colStatic.count();
    Collider* statBodies = colStatic.data();
    const i32 dynCount = bodies.count();
    PhysBody* dynBodies = bodies.data();

    for(i32 sb = 0; sb < statCount; sb++) {
        Collider col = statBodies[sb];
        col.setPos(col.getPos() + offset);
        colliderDbgDraw(col, vec4{0.0f, 200.f/255.f, 1.0, 0.5f});
    }

    for(i32 db = 0; db < dynCount; db++) {
        Collider col = dynBodies[db].col;
        col.setPos(col.getPos() + offset);
        colliderDbgDraw(dynBodies[db].col, vec4{1.0f, 200.f/255.f, 0, 0.5f});
    }
}
