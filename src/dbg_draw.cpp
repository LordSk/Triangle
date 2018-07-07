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
    bgfx::ProgramHandle progDbgColorInstance;
    bgfx::VertexBufferHandle cubeVbh;

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

        return true;
    }

    void deinit() {
        bgfx::destroy(cubeVbh);
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

    void render() {
        const i32 cubeCount = instDataObb.size();
        if(!cubeCount) {
            return;
        }

        // 80 bytes stride = 64 bytes for 4x4 matrix + 16 bytes for RGBA color.
        const uint16_t instanceStride = 80;
        const uint32_t numInstances   = cubeCount;

        if(numInstances == bgfx::getAvailInstanceDataBuffer(numInstances, instanceStride)) {
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

            bgfx::submit(0, progDbgColorInstance);
        }

        instDataObb.clear();
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
