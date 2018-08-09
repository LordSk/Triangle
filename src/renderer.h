#pragma once

#include "vector_math.h"
#include "mesh_load.h"
#include "utils.h"
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

struct InstanceData
{
    mat4 mtxModel;
    vec4 color;
};

struct Camera
{
    vec3 eye = {0, 0, 0};
    vec3 at  = {0, 0.001f, -1.f};
    vec3 up  = {0, 0, 1.f};
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
    bgfx::ProgramHandle progShadow;
    bgfx::ProgramHandle progShadowInstance;

    bgfx::UniformHandle u_color;
    bgfx::UniformHandle u_sdiffuse;
    bgfx::UniformHandle u_depthScaleOffset;

    bgfx::VertexBufferHandle cubeVbh;
    bgfx::VertexBufferHandle originVbh;
    bgfx::VertexBufferHandle gridVbh;
    bgfx::VertexBufferHandle vbhScreenQuad;

    bgfx::FrameBufferHandle fbhGame;
    bgfx::FrameBufferHandle fbhUI;
    bgfx::FrameBufferHandle fbhShadowMap;

    bgfx::TextureHandle texShadowMap;

    bool8 caps_shadowSamplerSupported;

    mat4 mtxProj;
    mat4 mtxView;

    Array<Camera> cameraList;
    i32 currentCamId = 0;

    bool dbgEnableGrid = true;
    bool dbgEnableWorldOrigin = true;
    bool dbgEnableDbgPhysics = false;


    bool init(i32 renderWidth_, i32 renderHeight_);
    void deinit();

    void setView(const mat4& proj, const mat4& view);
    void frame();

    void drawMesh(MeshHandle hmesh, const mat4& mtxModel, const vec4& color);
    void drawCubeInstances(const InstanceData* instData, const i32 cubeCount, const bool8 dropShadow);

    void setCamera(const i32 camId, Camera cam);
    void selectCamera(const i32 camId);
};

void setRendererGlobal(Renderer* renderer);
Renderer& getRenderer();
