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
