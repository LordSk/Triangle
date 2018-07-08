#include "collision.h"
#include "dbg_draw.h"

// sepearatig axis theorem
bool obbIntersectObb(const OrientedBoundingBox& obbA, const OrientedBoundingBox& obbB, void* out)
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
    f32 dxA = bx::abs(vec2Dot(T, xA));
    f32 cmp = halfWidthA + bx::abs(vec2Dot(xB * halfWidthB, xA)) + bx::abs(vec2Dot(yB * halfHeightB, xA));
    if(dxA > cmp) {
        return false;
    }

    //| T • Ay | > HA + | ( WB*Bx ) • Ay | + |( HB*By ) • Ay |
    f32 dyA = bx::abs(vec2Dot(T, yA));
    cmp = halfHeightA + bx::abs(vec2Dot(xB * halfWidthB, yA)) + bx::abs(vec2Dot(yB * halfHeightB, yA));
    if(dyA > cmp) {
        return false;
    }

    //| T • Bx | > | ( WA* Ax ) • Bx | + | ( HA*Ay ) • Bx | + WB
    f32 dxB = bx::abs(vec2Dot(T, xB));
    cmp = halfWidthB + bx::abs(vec2Dot(xA * halfWidthA, xB)) + bx::abs(vec2Dot(yA * halfHeightA, xB));
    if(dxB > cmp) {
        return false;
    }

    //| T • By | > | ( WA* Ax ) • By | + | ( HA*Ay ) • By | + HB
    f32 dyB = bx::abs(vec2Dot(T, yB));
    cmp = halfHeightB + bx::abs(vec2Dot(xA * halfWidthA, yB)) + bx::abs(vec2Dot(yA * halfHeightA, yB));
    if(dyB > cmp) {
        return false;
    }

    return true;
}

bool obbIntersectCb(const OrientedBoundingBox& obbA, const CircleBound& cbB, void* out)
{
    return true;
}

void obbDbgDraw(const OrientedBoundingBox& obb, vec4 color)
{
    const f32 cosA = bx::cos(obb.angle);
    const f32 sinA = bx::sin(obb.angle);
    vec3 xA = vec3{ cosA * 5, -sinA * 5, 0 };
    vec3 yA = vec3{ sinA * 5, cosA * 5, 0 };

    quat q;
    bx::quatRotateZ(q, obb.angle);
    // continue here
    vec3 pmax;
    bx::vec3MulQuat(pmax, obb.size, q);

    vec3 centerA = obb.origin + pmax * 0.5f;

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
