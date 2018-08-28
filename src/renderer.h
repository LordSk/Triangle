#pragma once

#include "vector_math.h"
#include "mesh_load.h"
#include "utils.h"
#include <bgfx/bgfx.h>

#define SHADOW_MAP_COUNT_MAX 8

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

struct ShadowMapDirectional
{
    vec3 pos;
    vec3 dir;
    vec3 orthoMin;
    vec3 orthoMax;
    AABB worldArea;
};

struct LightPoint
{
    vec3 pos = {};
    vec3 color1 = {1, 1, 1};
    vec3 color2 = {0, 0, 0};
    f32 intensity = 1.0f;
    f32 att_linear = 0.1f;
    f32 att_quadratic = 0.1f;
};

struct Renderer
{
    struct ViewID
    {
        enum Enum {
            SHADOW_MAP0 = 0,
            SHADOW_MAP1,
            SHADOW_MAP2,
            SHADOW_MAP3,
            SHADOW_MAP4,
            SHADOW_MAP5,
            SHADOW_MAP6,
            SHADOW_MAP7,

            GAME,
            LIGHT,
            COMBINE,
            POST_PROCESS,
            DBG_DRAW,
            UI,
        };
    };

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
    bgfx::ProgramHandle progMeshShadowed;
    bgfx::ProgramHandle progMeshShadowedInstance;
    bgfx::ProgramHandle progGbuffer;
    bgfx::ProgramHandle progGbufferInst;
    bgfx::ProgramHandle progLightPass;
    bgfx::ProgramHandle progToneMap;

    bgfx::UniformHandle u_color;
    bgfx::UniformHandle s_albedo;
    bgfx::UniformHandle s_position;
    bgfx::UniformHandle s_normal;
    bgfx::UniformHandle s_depth;
    bgfx::UniformHandle u_depthScaleOffset;
    bgfx::UniformHandle s_shadowMap;
    bgfx::UniformHandle s_lightMap;
    bgfx::UniformHandle s_combine;
    bgfx::UniformHandle u_lightPos;
    bgfx::UniformHandle u_lightMtx;
    bgfx::UniformHandle u_lightDir;
    bgfx::UniformHandle u_lightColor1;
    bgfx::UniformHandle u_lightColor2;
    bgfx::UniformHandle u_lightLinearQuadraticIntensity;
    bgfx::UniformHandle u_exposure;

    bgfx::VertexBufferHandle cubeVbh;
    bgfx::VertexBufferHandle originVbh;
    bgfx::VertexBufferHandle gridVbh;
    bgfx::VertexBufferHandle vbhScreenQuad;

    bgfx::FrameBufferHandle fbhGbuffer;

    bgfx::FrameBufferHandle fbhShadowMap[SHADOW_MAP_COUNT_MAX];
    bgfx::TextureHandle texShadowMap[SHADOW_MAP_COUNT_MAX];

    bgfx::TextureHandle fbTexLight;
    bgfx::FrameBufferHandle fbhLight;

    bgfx::TextureHandle fbTexCombine;
    bgfx::FrameBufferHandle fbhCombine;

    bool8 caps_shadowSamplerSupported;

    mat4 mtxProj;
    mat4 mtxView;
    mat4 mtxLight0;

    Array<Camera> cameraList;
    i32 currentCamId = 0;

    Array<LightPoint> lightPointList;

    bool dbgEnableGrid = false;
    bool dbgEnableWorldOrigin = false;
    bool dbgEnableDbgPhysics = false;
    f32 dbgExposure = 1.0f;

    bool init(i32 renderWidth_, i32 renderHeight_);
    void deinit();

    void setView(const mat4& proj, const mat4& view);
    void frame();

    void drawMesh(MeshHandle hmesh, const mat4& mtxModel, const vec4& color);
    void drawCubeInstances(const InstanceData* instData, const i32 cubeCount, const bool8 dropShadow);
    void drawCube(mat4 mtxModel, vec4 color);

    void setCamera(const i32 camId, Camera cam);
    void selectCamera(const i32 camId);

    void fitShadowMapToSceneBounds(ShadowMapDirectional* light);
    void setupDirectionalShadowMap(const ShadowMapDirectional& light, const i32 shadowMapId);
};

void setRendererGlobal(Renderer* renderer);
Renderer& getRenderer();
