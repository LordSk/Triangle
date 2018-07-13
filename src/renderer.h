#pragma once

#include "vector_math.h"
#include "mesh_load.h"
#include <bgfx/bgfx.h>

// TODO: move this
struct PosColorVertex
{
    f32 x, y, z;
    u32 color_abgr;
    f32 nx, ny, nz;

    static void init()
    {
        ms_decl
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
            .add(bgfx::Attrib::Normal,   3, bgfx::AttribType::Float)
            .end();
    }

    static bgfx::VertexDecl ms_decl;
};

struct PosUvVertex
{
    f32 x, y, z;
    f32 u, v;

    static void init()
    {
        decl
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0,2, bgfx::AttribType::Float)
            .end();
    }

    static bgfx::VertexDecl decl;
};

struct RdrViewID
{
    enum {
        GAME = 0,
        UI,
        COMBINE,
    } Enum;
};

struct InstanceData
{
    mat4 mtxModel;
    vec4 color;
};

struct Renderer
{
    i32 renderWidth;
    i32 renderHeight;

    bgfx::ProgramHandle progTest;
    bgfx::ProgramHandle progVertShading;
    bgfx::ProgramHandle progVertShadingColor;
    bgfx::ProgramHandle progVertShadingColorInstance;
    bgfx::ProgramHandle progDbgColor;
    bgfx::ProgramHandle progGameFinal;
    bgfx::ProgramHandle progUiFinal;

    bgfx::UniformHandle u_color;
    bgfx::UniformHandle u_sdiffuse;

    bgfx::VertexBufferHandle cubeVbh;
    bgfx::VertexBufferHandle originVbh;
    bgfx::VertexBufferHandle gridVbh;
    bgfx::VertexBufferHandle vbhScreenQuad;

    bgfx::FrameBufferHandle fbhGame;
    bgfx::FrameBufferHandle fbhUI;

    mat4 mtxProj;
    mat4 mtxView;

    bool dbgEnableGrid = true;
    bool dbgEnableWorldOrigin = true;
    bool dbgEnableDbgPhysics = false;


    bool init(i32 renderWidth_, i32 renderHeight_);
    void deinit();

    void setView(const mat4& proj, const mat4& view);
    void frame();

    void drawMesh(MeshHandle hmesh, const mat4& mtxModel, const vec4& color);
    void drawCubeInstances(const InstanceData* instData, const i32 cubeCount);
};

void setRendererGlobal(Renderer* renderer);
Renderer& getRenderer();