#include "dbg_draw.h"
#include "utils.h"
#include "static_mesh.h"

static PosColorVertex s_cubeVertData[] =
{
    // left
    { 0.0f, 0.0f, 0.0f, 0xffffff00, -1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 1.0f, 0xff000000, -1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 0xff00ff00, -1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0xffffff00, -1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0xffff00ff, -1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 1.0f, 0xff000000, -1.0f, 0.0f, 0.0f },


    // right
    { 1.0f, 1.0f, 1.0f, 0xff0000ff, 1.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f, 0xffff00ff, 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f, 0xffffffff, 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f, 0xffffffff, 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 1.0f, 0xff00ffff, 1.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f, 0xff0000ff, 1.0f, 0.0f, 0.0f },


    // bottom
    { 1.0f, 1.0f, 0.0f, 0xffff00ff, 0.0f, 0.0f, -1.0f },
    { 0.0f, 1.0f, 0.0f, 0xffff00ff, 0.0f, 0.0f, -1.0f },
    { 0.0f, 0.0f, 0.0f, 0xffffff00, 0.0f, 0.0f, -1.0f },
    { 1.0f, 1.0f, 0.0f, 0xffff00ff, 0.0f, 0.0f, -1.0f },
    { 0.0f, 0.0f, 0.0f, 0xffffff00, 0.0f, 0.0f, -1.0f },
    { 1.0f, 0.0f, 0.0f, 0xffffffff, 0.0f, 0.0f, -1.0f },


    // top
    { 0.0f, 1.0f, 1.0f, 0xff000000, 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f, 0xff00ffff, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 0xff00ff00, 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 0xff0000ff, 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f, 0xff00ffff, 0.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f, 0xff000000, 0.0f, 0.0f, 1.0f },


    // front
    { 1.0f, 0.0f, 1.0f, 0xff00ffff, 0.0f, -1.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f, 0xffffffff, 0.0f, -1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0xffffff00, 0.0f, -1.0f, 0.0f },
    { 1.0f, 0.0f, 1.0f, 0xff00ffff, 0.0f, -1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0xffffff00, 0.0f, -1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 0xff00ff00, 0.0f, -1.0f, 0.0f },


    // back
    { 1.0f, 1.0f, 1.0f, 0xff0000ff, 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0xffff00ff, 0.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f, 0xffff00ff, 0.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f, 0xff0000ff, 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 1.0f, 0xff000000, 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0xffff00ff, 0.0f, 1.0f, 0.0f },
};

static bx::FileReader g_fileReader;

struct DbgDraw
{
    struct InstanceData {
        mat4 mtxModel;
        vec4 color;
    };

    Array<InstanceData> instDataObb;
    Array<InstanceData> instDataSphere;
    bgfx::ProgramHandle progDbgColorInstance;
    bgfx::VertexBufferHandle cubeVbh;
    bgfx::VertexBufferHandle sphereVbh;
    i32 sphereVertCount;

    bool init() {
        progDbgColorInstance = loadProgram(&g_fileReader, "vs_dbg_color_instance",
                                                          "fs_dbg_color_instance");
        if(!bgfx::isValid(progDbgColorInstance)) {
            return false;
        }

        cubeVbh = bgfx::createVertexBuffer(
                    bgfx::makeRef(s_cubeVertData, sizeof(s_cubeVertData)),
                    PosColorVertex::ms_decl
                    );

        instDataObb.reserve(2048);
        instDataSphere.reserve(2048);

        static PosColorVertex sphereVertData[8192];
        makeSphere(sphereVertData, arr_count(sphereVertData), &sphereVertCount);

        sphereVbh = bgfx::createVertexBuffer(
                    bgfx::makeRef(sphereVertData, sizeof(sphereVertData)),
                    PosColorVertex::ms_decl
                    );

        return true;
    }

    void deinit() {
        bgfx::destroy(cubeVbh);
        bgfx::destroy(sphereVbh);
        bgfx::destroy(progDbgColorInstance);
    }

    void obb(const Transform& tf, vec4 color) {
        mat4 mtx;
        tf.toMtx(&mtx);
        instDataObb.push({ mtx, color });
    }

    void line(const vec3& p1, const vec3& p2, vec4 color, f32 thickness) {
        vec3 delta = p2 - p1;
        quat rot = vec3RotDiffQuat({ 1, 0, 0 }, delta);
        f32 len = bx::vec3Length(delta);

        Transform tf;
        tf.pos = p1;
        tf.scale = { len, thickness, thickness };
        tf.rot = rot;

        mat4 mtx;
        tf.toMtx(&mtx);
        instDataObb.push({ mtx, color });
    }

    void sphere(const vec3& pos, f32 radius, vec4 color) {
        Transform tf;
        tf.pos = pos;
        tf.scale = { radius, radius, radius };

        mat4 mtx;
        tf.toMtx(&mtx);
        instDataSphere.push({ mtx, color });
    }

    void render() {
        const i32 cubeCount = instDataObb.count();
        const i32 sphereCount = instDataSphere.count();
        if(!cubeCount && !sphereCount) {
            return;
        }

        // 80 bytes stride = 64 bytes for 4x4 matrix + 16 bytes for RGBA color.
        const u16 instanceStride = 80;

        u32 numInstances = cubeCount;

        if(numInstances > 0 &&
           numInstances == bgfx::getAvailInstanceDataBuffer(numInstances, instanceStride)) {
            bgfx::InstanceDataBuffer idb;
            bgfx::allocInstanceDataBuffer(&idb, numInstances, instanceStride);

            memmove(idb.data, instDataObb.data(), sizeof(InstanceData) * cubeCount);

            bgfx::setVertexBuffer(0, cubeVbh, 0, BX_COUNTOF(s_cubeVertData));
            bgfx::setInstanceDataBuffer(&idb);

            bgfx::setState(0
                | BGFX_STATE_WRITE_MASK
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
                | BGFX_STATE_BLEND_ALPHA
                );

            bgfx::submit(Renderer::ViewID::GAME, progDbgColorInstance);
        }

        instDataObb.clear();

        numInstances = sphereCount;

        if(numInstances > 0 &&
           numInstances == bgfx::getAvailInstanceDataBuffer(numInstances, instanceStride)) {
            bgfx::InstanceDataBuffer idb;
            bgfx::allocInstanceDataBuffer(&idb, numInstances, instanceStride);

            memmove(idb.data, instDataSphere.data(), sizeof(InstanceData) * sphereCount);

            bgfx::setVertexBuffer(0, sphereVbh, 0, sphereVertCount);
            bgfx::setInstanceDataBuffer(&idb);

            bgfx::setState(0
                | BGFX_STATE_WRITE_MASK
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
                | BGFX_STATE_BLEND_ALPHA
                );

            bgfx::submit(Renderer::ViewID::GAME, progDbgColorInstance);
        }

        instDataSphere.clear();
    }
};

static DbgDraw g_dbgDraw;

bool dbgDrawInit()
{
    return g_dbgDraw.init();
}

void dbgDrawDeinit()
{
    g_dbgDraw.deinit();
}

void dbgDrawRect(const Transform& tf, vec4 color)
{
    g_dbgDraw.obb(tf, color);
}
void dbgDrawRectLine(const Transform& tf, vec4 color, f32 thick)
{
    mat4 mtxTf;
    tf.toMtx(&mtxTf);

    vec3 pmin = {0, 0, 0};
    vec3 pmax = {1, 1, 1};

    vec3 ftl = pmin;
    vec3 ftr = {pmax.x, pmin.y, pmin.z};
    vec3 fbl = {pmin.x, pmax.y, pmin.z};
    vec3 fbr = {pmax.x, pmax.y, pmin.z};

    vec3 btl = {pmin.x, pmin.y, pmax.z};
    vec3 btr = {pmax.x, pmin.y, pmax.z};
    vec3 bbl = {pmin.x, pmax.y, pmax.z};
    vec3 bbr = pmax;

    bx::vec3MulMtxH(ftl, ftl, mtxTf);
    bx::vec3MulMtxH(ftr, ftr, mtxTf);
    bx::vec3MulMtxH(fbl, fbl, mtxTf);
    bx::vec3MulMtxH(fbr, fbr, mtxTf);

    bx::vec3MulMtxH(btl, btl, mtxTf);
    bx::vec3MulMtxH(btr, btr, mtxTf);
    bx::vec3MulMtxH(bbl, bbl, mtxTf);
    bx::vec3MulMtxH(bbr, bbr, mtxTf);

    g_dbgDraw.line(ftl, ftr, color, thick);
    g_dbgDraw.line(fbl, fbr, color, thick);
    g_dbgDraw.line(ftl, fbl, color, thick);
    g_dbgDraw.line(ftr, fbr, color, thick);

    g_dbgDraw.line(btl, btr, color, thick);
    g_dbgDraw.line(bbl, bbr, color, thick);
    g_dbgDraw.line(btl, bbl, color, thick);
    g_dbgDraw.line(btr, bbr, color, thick);

    g_dbgDraw.line(ftl, btl, color, thick);
    g_dbgDraw.line(ftr, btr, color, thick);
    g_dbgDraw.line(fbl, bbl, color, thick);
    g_dbgDraw.line(fbr, bbr, color, thick);
}

void dbgDrawLine(const vec3& p1, const vec3& p2, vec4 color, f32 thickness)
{
    g_dbgDraw.line(p1, p2, color, thickness);
}

void dbgDrawRender()
{
    g_dbgDraw.render();
}

void dbgDrawSphere(const vec3& pos, f32 radius, vec4 color)
{
    g_dbgDraw.sphere(pos, radius, color);
}

void dbgDrawOrthoFrustrum(const vec3& eye, const vec3& at, f32 near, f32 far, f32 left,
                          f32 right, f32 top, f32 bottom, vec4 color, f32 thick)
{
    vec3 dir = vec3Norm(at - eye);
    vec3 up = {0, 0, 1};
    vec3 vright;
    bx::vec3Cross(vright, dir, up);
    vright = vec3Norm(vright);
    bx::vec3Cross(up, vright, dir);
    up = vec3Norm(up);

    const vec3 ftl = eye + (vright * left) + up * top + dir * near;
    const vec3 ftr = eye + (vright * right) + up * top + dir * near;
    const vec3 fbl = eye + (vright * left) + up * bottom + dir * near;
    const vec3 fbr = eye + (vright * right) + up * bottom + dir * near;

    const vec3 btl = eye + (vright * left) + up * top + dir * far;
    const vec3 btr = eye + (vright * right) + up * top + dir * far;
    const vec3 bbl = eye + (vright * left) + up * bottom + dir * far;
    const vec3 bbr = eye + (vright * right) + up * bottom + dir * far;

    g_dbgDraw.line(ftl, ftr, color, thick);
    g_dbgDraw.line(fbl, fbr, color, thick);
    g_dbgDraw.line(ftl, fbl, color, thick);
    g_dbgDraw.line(ftr, fbr, color, thick);

    g_dbgDraw.line(btl, btr, color, thick);
    g_dbgDraw.line(bbl, bbr, color, thick);
    g_dbgDraw.line(btl, bbl, color, thick);
    g_dbgDraw.line(btr, bbr, color, thick);

    g_dbgDraw.line(ftl, btl, color, thick);
    g_dbgDraw.line(ftr, btr, color, thick);
    g_dbgDraw.line(fbl, bbl, color, thick);
    g_dbgDraw.line(fbr, bbr, color, thick);
}

void dbgDrawOrthoFrustrumSolid(const vec3& eye, const vec3& at, f32 near, f32 far, f32 left,
                               f32 right, f32 top, f32 bottom, vec4 color, f32 thick)
{
    vec3 dir = vec3Norm(at - eye);
    vec3 up = {0, 0, 1};
    vec3 vright;
    bx::vec3Cross(vright, dir, up);
    bx::vec3Cross(up, vright, dir);

    const vec3 ftl = eye + (vright * left) + up * top + dir * near;
    const vec3 ftr = eye + (vright * right) + up * top + dir * near;
    const vec3 fbl = eye + (vright * left) + up * bottom + dir * near;
    const vec3 fbr = eye + (vright * right) + up * bottom + dir * near;

    const vec3 btl = eye + (vright * left) + up * top + dir * far;
    const vec3 btr = eye + (vright * right) + up * top + dir * far;
    const vec3 bbl = eye + (vright * left) + up * bottom + dir * far;
    const vec3 bbr = eye + (vright * right) + up * bottom + dir * far;


    g_dbgDraw.line(ftl, ftr, color, thick);
    g_dbgDraw.line(fbl, fbr, color, thick);
    g_dbgDraw.line(ftl, fbl, color, thick);
    g_dbgDraw.line(ftr, fbr, color, thick);

    g_dbgDraw.line(btl, btr, color, thick);
    g_dbgDraw.line(bbl, bbr, color, thick);
    g_dbgDraw.line(btl, bbl, color, thick);
    g_dbgDraw.line(btr, bbr, color, thick);

    g_dbgDraw.line(ftl, btl, color, thick);
    g_dbgDraw.line(ftr, btr, color, thick);
    g_dbgDraw.line(fbl, bbl, color, thick);
    g_dbgDraw.line(fbr, bbr, color, thick);
    assert(0); // TODO: implement
}
