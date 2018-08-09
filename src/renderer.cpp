#include "renderer.h"
#include "utils.h"
#include <bgfx/bgfx.h>
#include <bx/timer.h>

struct ViewID
{
    enum Enum {
        GAME = 0,
        UI,
        COMBINE,
        SHADOW_MAP0,
    };
};

// TODO: do not use this
static bx::FileReader g_fileReader;
static bx::FileWriter g_fileWriter;

bgfx::VertexDecl PosColorVertex::ms_decl;
bgfx::VertexDecl PosUvVertex::decl;

static const PosUvVertex s_screenQuadVertData[6] = {
    { -1.0f, -1.0f, 0, 0, 1 },
    {  1.0f, -1.0f, 0, 1, 1 },
    {  1.0f,  1.0f, 0, 1, 0 },

    { -1.0f, -1.0f, 0, 0, 1 },
    {  1.0f,  1.0f, 0, 1, 0 },
    { -1.0f,  1.0f, 0, 0, 0 },
};

static PosColorVertex s_originVertData[] =
{
    {0, 0, 0, 0xffff0000 },
    {10, 0, 0, 0xffff0000 },

    {0, 0, 0, 0xff00ff00 },
    {0, 10, 0, 0xff00ff00 },

    {0, 0, 0, 0xff0000ff },
    {0, 0, 10, 0xff0000ff },
};

static PosColorVertex s_cubeRainbowVertData[] =
{
    // left
    {-0.5f, -0.5f, -0.5f, 0xffffff00, -1.0f, 0.0f, 0.0f },
    {-0.5f,  0.5f,  0.5f, 0xff000000, -1.0f, 0.0f, 0.0f },
    {-0.5f, -0.5f,  0.5f, 0xff00ff00, -1.0f, 0.0f, 0.0f },
    {-0.5f, -0.5f, -0.5f, 0xffffff00, -1.0f, 0.0f, 0.0f },
    {-0.5f,  0.5f, -0.5f, 0xffff00ff, -1.0f, 0.0f, 0.0f },
    {-0.5f,  0.5f,  0.5f, 0xff000000, -1.0f, 0.0f, 0.0f },


    // right
    { 0.5f,  0.5f,  0.5f, 0xff0000ff, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f, -0.5f, 0xffff00ff, 1.0f, 0.0f, 0.0f },
    { 0.5f, -0.5f, -0.5f, 0xffffffff, 1.0f, 0.0f, 0.0f },
    { 0.5f, -0.5f, -0.5f, 0xffffffff, 1.0f, 0.0f, 0.0f },
    { 0.5f, -0.5f,  0.5f, 0xff00ffff, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  0.5f, 0xff0000ff, 1.0f, 0.0f, 0.0f },


    // bottom
    { 0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 0.0f, -1.0f },
    {-0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 0.0f, -1.0f },
    {-0.5f, -0.5f, -0.5f, 0xffffff00, 0.0f, 0.0f, -1.0f },
    { 0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 0.0f, -1.0f },
    {-0.5f, -0.5f, -0.5f, 0xffffff00, 0.0f, 0.0f, -1.0f },
    { 0.5f, -0.5f, -0.5f, 0xffffffff, 0.0f, 0.0f, -1.0f },


    // top
    {-0.5f,  0.5f,  0.5f, 0xff000000, 0.0f, 0.0f, 1.0f },
    { 0.5f, -0.5f,  0.5f, 0xff00ffff, 0.0f, 0.0f, 1.0f },
    {-0.5f, -0.5f,  0.5f, 0xff00ff00, 0.0f, 0.0f, 1.0f },
    { 0.5f,  0.5f,  0.5f, 0xff0000ff, 0.0f, 0.0f, 1.0f },
    { 0.5f, -0.5f,  0.5f, 0xff00ffff, 0.0f, 0.0f, 1.0f },
    {-0.5f,  0.5f,  0.5f, 0xff000000, 0.0f, 0.0f, 1.0f },


    // front
    { 0.5f, -0.5f,  0.5f, 0xff00ffff, 0.0f, -1.0f, 0.0f },
    { 0.5f, -0.5f, -0.5f, 0xffffffff, 0.0f, -1.0f, 0.0f },
    {-0.5f, -0.5f, -0.5f, 0xffffff00, 0.0f, -1.0f, 0.0f },
    { 0.5f, -0.5f,  0.5f, 0xff00ffff, 0.0f, -1.0f, 0.0f },
    {-0.5f, -0.5f, -0.5f, 0xffffff00, 0.0f, -1.0f, 0.0f },
    {-0.5f, -0.5f,  0.5f, 0xff00ff00, 0.0f, -1.0f, 0.0f },


    // back
    { 0.5f,  0.5f,  0.5f, 0xff0000ff, 0.0f, 1.0f, 0.0f },
    {-0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 1.0f, 0.0f },
    { 0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 1.0f, 0.0f },
    { 0.5f,  0.5f,  0.5f, 0xff0000ff, 0.0f, 1.0f, 0.0f },
    {-0.5f,  0.5f,  0.5f, 0xff000000, 0.0f, 1.0f, 0.0f },
    {-0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 1.0f, 0.0f },
};

static PosColorVertex s_gridLinesVertData[400];

static Renderer* g_rdr;

void setRendererGlobal(Renderer* renderer)
{
    g_rdr = renderer;
}

Renderer& getRenderer()
{
    return *g_rdr;
}

bool Renderer::init(i32 renderWidth_, i32 renderHeight_)
{
    renderWidth = renderWidth_;
    renderHeight = renderHeight_;

    cameraList.resize(16);

    bgfx::Init init;
    init.type = bgfx::RendererType::Direct3D11;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.deviceId = 0;
    init.resolution.width  = renderWidth;
    init.resolution.height = renderHeight;
    init.resolution.reset = BGFX_RESET_NONE;

    if(!bgfx::init(init)) {
        LOG("ERROR> bgfx failed to initialize");
        return false;
    }

    bgfx::setDebug(BGFX_DEBUG_TEXT);

    const u32 samplerFlags = 0
                    | BGFX_TEXTURE_RT
                    | BGFX_TEXTURE_MIN_POINT
                    | BGFX_TEXTURE_MAG_POINT
                    | BGFX_TEXTURE_MIP_POINT
                    | BGFX_TEXTURE_U_CLAMP
                    | BGFX_TEXTURE_V_CLAMP
                    ;

    // TODO: fix depth buffer multisampling
    // https://github.com/bkaradzic/bgfx/issues/1353
    bgfx::TextureHandle fbTexGame[2];
    fbTexGame[0] = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::BGRA8,
                                         samplerFlags|BGFX_TEXTURE_SRGB/*|BGFX_TEXTURE_RT_MSAA_X16*/);
    fbTexGame[1] = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::D24, samplerFlags/*|BGFX_TEXTURE_RT_MSAA_X16*/);
    fbhGame = bgfx::createFrameBuffer(2, fbTexGame, true);

    if(!bgfx::isValid(fbhGame)) {
        LOG("ERROR> could not create game frame buffer");
        return false;
    }

    fbhUI = bgfx::createFrameBuffer(
                  renderWidth, renderHeight,
                  bgfx::TextureFormat::RGBA8,
                  (samplerFlags|BGFX_TEXTURE_RT_MSAA_X16));

    if(!bgfx::isValid(fbhUI)) {
        LOG("ERROR> could not create UI frame buffer");
        return false;
    }

    // Set view clear state.
    bgfx::setViewClear(ViewID::GAME, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, 0x101010ff, 1.0f, 0.f);
    bgfx::setViewClear(ViewID::UI, BGFX_CLEAR_COLOR, 0x00000000, 1.f, 0.f);
    bgfx::setViewClear(ViewID::COMBINE, BGFX_CLEAR_COLOR, 0x101010ff, 1.0f, 0.f);

    bgfx::setViewFrameBuffer(ViewID::GAME, fbhGame);
    bgfx::setViewFrameBuffer(ViewID::UI, fbhUI);

    bgfx::setViewName(ViewID::GAME, "Game");
    bgfx::setViewName(ViewID::UI, "ImGui");
    bgfx::setViewName(ViewID::COMBINE, "Combine");

    // Create vertex stream declaration.
    PosColorVertex::init();
    PosUvVertex::init();

    // Create static vertex buffer.
    cubeVbh = bgfx::createVertexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_cubeRainbowVertData, sizeof(s_cubeRainbowVertData)),
            PosColorVertex::ms_decl
            );
    originVbh = bgfx::createVertexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_originVertData, sizeof(s_originVertData)),
            PosColorVertex::ms_decl
            );

    vbhScreenQuad = bgfx::createVertexBuffer(
                        bgfx::makeRef(s_screenQuadVertData, sizeof(s_screenQuadVertData)),
                        PosUvVertex::decl
                        );

    u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
    u_sdiffuse = bgfx::createUniform("u_sdiffuse", bgfx::UniformType::Int1); // sampler2D

    progTest = loadProgram(&g_fileReader, "vs_test", "fs_test");
    progVertShading = loadProgram(&g_fileReader, "vs_vertex_shading", "fs_vertex_shading");
    progVertShadingColor = loadProgram(&g_fileReader, "vs_vertex_shading", "fs_vertex_shading_color");
    progVertShadingColorInstance = loadProgram(&g_fileReader, "vs_vertex_shading_instance",
                                                  "fs_vertex_shading_instance");
    progDbgColor = loadProgram(&g_fileReader, "vs_dbg_color", "fs_dbg_color");
    progGameFinal = loadProgram(&g_fileReader, "vs_game_final", "fs_game_final");
    progUiFinal = loadProgram(&g_fileReader, "vs_game_final", "fs_ui_final");


    i32 i = 0;
    for(i32 x = 0; x < 100; x++) {
        s_gridLinesVertData[i++] = { x * 10.0f, 0, 0 };
        s_gridLinesVertData[i++] = { x * 10.0f, 1000.0f, 0 };
    }
    for(i32 y = 0; y < 100; y++) {
        s_gridLinesVertData[i++] = { 0, y * 10.0f, 0 };
        s_gridLinesVertData[i++] = { 1000.0f, y * 10.0f, 0 };
    }

    gridVbh = bgfx::createVertexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_gridLinesVertData, sizeof(s_gridLinesVertData)),
            PosColorVertex::ms_decl
            );

    const bgfx::Caps& caps = *bgfx::getCaps();
    caps_shadowSamplerSupported = (caps.supported & BGFX_CAPS_TEXTURE_COMPARE_LEQUAL) != 0;

    const i32 shadowMapSize = 2048;

    if(caps_shadowSamplerSupported) {
        progShadow = loadProgram(&g_fileReader, "vs_shadow_leq", "fs_shadow_leq");
        progShadowInstance = loadProgram(&g_fileReader, "vs_shadow_leq_instance", "fs_shadow_leq");

        bgfx::TextureHandle fbtextures[] =
        {
            bgfx::createTexture2D(shadowMapSize, shadowMapSize, false,
                1, bgfx::TextureFormat::D16, BGFX_TEXTURE_RT | BGFX_TEXTURE_COMPARE_LEQUAL),
        };
        texShadowMap = fbtextures[0];
        fbhShadowMap = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
    }
    else {
        LOG("Renderer> shadow samplers are not supported.");

        progShadow = loadProgram(&g_fileReader, "vs_shadow", "fs_shadow");
        progShadowInstance = loadProgram(&g_fileReader, "vs_shadow_instance", "fs_shadow");

        bgfx::TextureHandle fbtextures[] =
        {
            bgfx::createTexture2D(shadowMapSize, shadowMapSize,
                false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT),

            bgfx::createTexture2D(shadowMapSize, shadowMapSize,
                false, 1, bgfx::TextureFormat::D16, BGFX_TEXTURE_RT_WRITE_ONLY),
        };

        texShadowMap = fbtextures[0];
        fbhShadowMap = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
    }

    if(!bgfx::isValid(fbhShadowMap)) {
        LOG("ERROR> could not create shadow frame buffer");
        return false;
    }

    mat4 mtxLightProj, mtxLightView;
    Camera camLight;
    camLight.eye = { 10.0f, 10.0f, 50.0f };
    camLight.at  = { 10.0f, 10.0f, 0.0f };
    camLight.up  = { 0, 0, 1 };
    bx::mtxLookAtRh(mtxLightView, camLight.eye, camLight.at, camLight.up);

    const float area = 30.0f;
    bx::mtxOrthoRh(mtxLightProj, -area, area, -area, area, -100.0f, 100.0f, 0.0f, caps.homogeneousDepth);

    bgfx::setViewRect(ViewID::SHADOW_MAP0, 0, 0, shadowMapSize, shadowMapSize);
    bgfx::setViewFrameBuffer(ViewID::SHADOW_MAP0, fbhShadowMap);
    bgfx::setViewTransform(ViewID::SHADOW_MAP0, mtxLightView, mtxLightProj);
    bgfx::setViewName(ViewID::SHADOW_MAP0, "ShadowMap0");

    bgfx::setViewClear(ViewID::SHADOW_MAP0, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

    u_depthScaleOffset = bgfx::createUniform("u_depthScaleOffset",  bgfx::UniformType::Vec4);

    f32 depthScaleOffset[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
    if(caps.homogeneousDepth) {
        depthScaleOffset[0] = 0.5f;
        depthScaleOffset[1] = 0.5f;
    }
    bgfx::setUniform(u_depthScaleOffset, depthScaleOffset);

    return true;
}

void Renderer::deinit()
{
    bgfx::destroy(originVbh);
    bgfx::destroy(gridVbh);
    bgfx::destroy(cubeVbh);
    bgfx::destroy(vbhScreenQuad);

    bgfx::destroy(progTest);
    bgfx::destroy(progVertShading);
    bgfx::destroy(progVertShadingColor);
    bgfx::destroy(progVertShadingColorInstance);
    bgfx::destroy(progDbgColor);
    bgfx::destroy(progGameFinal);
    bgfx::destroy(progUiFinal);
    bgfx::destroy(progShadow);
    bgfx::destroy(progShadowInstance);

    bgfx::destroy(u_color);
    bgfx::destroy(u_sdiffuse);
    bgfx::destroy(u_depthScaleOffset);

    bgfx::destroy(fbhGame);
    bgfx::destroy(fbhUI);
    bgfx::destroy(fbhShadowMap);

    bgfx::shutdown();
}

void Renderer::setView(const mat4& proj, const mat4& view)
{
    mtxProj = proj;
    mtxView = view;
}

void Renderer::frame()
{
    bgfx::setViewRect(ViewID::GAME, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(ViewID::UI, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(ViewID::COMBINE, 0, 0, u16(renderWidth), u16(renderHeight));

    bgfx::setViewTransform(ViewID::GAME, mtxView, mtxProj);

    // shadow map
    mat4 mtxLightProj, mtxLightView;
    Camera camLight = cameraList[currentCamId];
    bx::mtxLookAtRh(mtxLightView, camLight.eye, camLight.at, camLight.up);

    const bgfx::Caps* caps = bgfx::getCaps();
    const float area = 30.0f;
    bx::mtxOrthoRh(mtxLightProj, -area, area, -area, area, -100.0f, 100.0f, 0.0f, caps->homogeneousDepth);
    bgfx::setViewTransform(ViewID::SHADOW_MAP0, mtxLightView, mtxLightProj);


    // This dummy draw call is here to make sure that view x is cleared
    // if no other draw calls are submitted to view x.
    bgfx::touch(ViewID::SHADOW_MAP0);
    bgfx::touch(ViewID::GAME);
    bgfx::touch(ViewID::UI);
    bgfx::touch(ViewID::COMBINE);

    // origin gizmo
    if(dbgEnableWorldOrigin) {
        bgfx::setState(0
            | BGFX_STATE_WRITE_MASK
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_MSAA
            | BGFX_STATE_PT_LINES
            );

        const f32 transparent[] = {0, 0, 0, 0};
        bgfx::setUniform(u_color, transparent);
        bgfx::setVertexBuffer(0, originVbh, 0, BX_COUNTOF(s_originVertData));
        bgfx::submit(ViewID::GAME, progDbgColor);
    }

    // XY grid
    if(dbgEnableGrid) {
        bgfx::setState(0
            | BGFX_STATE_WRITE_MASK
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_MSAA
            | BGFX_STATE_PT_LINES
            );

        mat4 mtxGrid;
        bx::mtxTranslate(mtxGrid, -500, -500, 0);
        bgfx::setTransform(mtxGrid);

        const f32 white[] = {0.5, 0.5, 0.5, 1};
        bgfx::setUniform(u_color, white);
        bgfx::setVertexBuffer(0, gridVbh, 0, BX_COUNTOF(s_gridLinesVertData));
        bgfx::submit(ViewID::GAME, progDbgColor);
    }

    static i64 timeOffset = bx::getHPCounter();
    f32 time = (f32)((bx::getHPCounter()-timeOffset)/f64(bx::getHPFrequency()));


    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        );

    bgfx::setVertexBuffer(0, vbhScreenQuad, 0, 6);

    bgfx::setTexture(0, u_sdiffuse, bgfx::getTexture(fbhGame, 0));
    bgfx::submit(ViewID::COMBINE, progGameFinal);

    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA)
        );

    bgfx::setVertexBuffer(0, vbhScreenQuad, 0, 6);

    bgfx::setTexture(0, u_sdiffuse, bgfx::getTexture(fbhUI, 0));
    bgfx::submit(ViewID::COMBINE, progUiFinal);

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}

void Renderer::drawMesh(MeshHandle hmesh, const mat4& mtxModel, const vec4& color)
{
    bgfx::setUniform(u_color, color);

    meshSubmit(hmesh, ViewID::GAME, progVertShadingColor,
               mtxModel, BGFX_STATE_MASK);

    u64 state = 0
        | (caps_shadowSamplerSupported ? 0 : BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A)
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CCW
        | BGFX_STATE_MSAA;

    meshSubmit(hmesh, ViewID::SHADOW_MAP0, progShadow,
               mtxModel, state);
}

void Renderer::drawCubeInstances(const InstanceData* instData, const i32 cubeCount, const bool8 dropShadow)
{
    // 80 bytes stride = 64 bytes for 4x4 matrix + 16 bytes for RGBA color.
    const u16 instanceStride = 80;
    const u32 numInstances   = cubeCount;

    if(numInstances == bgfx::getAvailInstanceDataBuffer(numInstances, instanceStride)) {
        bgfx::InstanceDataBuffer idb;
        bgfx::allocInstanceDataBuffer(&idb, numInstances, instanceStride);

        memmove(idb.data, instData, instanceStride * cubeCount);

        bgfx::setVertexBuffer(0, cubeVbh, 0, 36);
        bgfx::setInstanceDataBuffer(&idb);

        bgfx::setState(0
            | BGFX_STATE_WRITE_MASK
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CCW
            | BGFX_STATE_MSAA
            );

        bgfx::submit(ViewID::GAME, progVertShadingColorInstance);

        if(dropShadow) {
            bgfx::setVertexBuffer(0, cubeVbh, 0, 36);
            bgfx::setInstanceDataBuffer(&idb);

            bgfx::setState(0
               | (caps_shadowSamplerSupported ? 0 : BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A)
               | BGFX_STATE_WRITE_Z
               | BGFX_STATE_DEPTH_TEST_LESS
               | BGFX_STATE_CULL_CCW
               | BGFX_STATE_MSAA);

            bgfx::submit(ViewID::SHADOW_MAP0, progShadowInstance);
        }
    }
}

void Renderer::setCamera(const i32 camId, Camera cam)
{
    if(camId > cameraList.count()) {
        cameraList.resize(cameraList.count() * 2);
        assert(camId < cameraList.count());
    }
    cameraList[camId] = cam;
}

void Renderer::selectCamera(const i32 camId)
{
    assert(camId < cameraList.count());
    currentCamId = camId;
    const Camera& cam = cameraList[camId];
    bx::mtxLookAtRh(mtxView, cam.eye, cam.at, cam.up);
}
