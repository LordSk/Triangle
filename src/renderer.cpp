#include "renderer.h"
#include "utils.h"
#include <bgfx/bgfx.h>
#include <bx/timer.h>

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

    bgfx::Init init;
    init.type = bgfx::RendererType::Direct3D11;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.deviceId = 0;
    init.resolution.width  = 1600;
    init.resolution.height = 900;
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

    bgfx::TextureHandle fbTexGame[2];
    fbTexGame[0] = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::BGRA8,
                                         samplerFlags|BGFX_TEXTURE_SRGB|BGFX_TEXTURE_RT_MSAA_X16);
    fbTexGame[1] = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::D24, samplerFlags|BGFX_TEXTURE_RT_MSAA_X16);
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

    // Set view 0 clear state.
    bgfx::setViewClear(RdrViewID::GAME, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, 0x101010ff, 1.0f, 0.f);
    bgfx::setViewClear(RdrViewID::UI, BGFX_CLEAR_COLOR, 0x00000000, 1.f, 0.f);
    bgfx::setViewClear(RdrViewID::COMBINE, BGFX_CLEAR_COLOR, 0x101010ff, 1.0f, 0.f);

    bgfx::setViewFrameBuffer(RdrViewID::GAME, fbhGame);
    bgfx::setViewFrameBuffer(RdrViewID::UI, fbhUI);

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

    bgfx::destroy(u_color);
    bgfx::destroy(u_sdiffuse);

    bgfx::destroy(fbhGame);
    bgfx::destroy(fbhUI);

    bgfx::shutdown();
}

void Renderer::setView(const mat4& proj, const mat4& view)
{
    mtxProj = proj;
    mtxView = view;
}

void Renderer::frame()
{
    bgfx::setViewRect(RdrViewID::GAME, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(RdrViewID::UI, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(RdrViewID::COMBINE, 0, 0, u16(renderWidth), u16(renderHeight));

    bgfx::setViewTransform(RdrViewID::GAME, mtxView, mtxProj);

    // This dummy draw call is here to make sure that view x is cleared
    // if no other draw calls are submitted to view x.
    bgfx::touch(RdrViewID::GAME);
    bgfx::touch(RdrViewID::UI);
    bgfx::touch(RdrViewID::COMBINE);

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
        bgfx::submit(RdrViewID::GAME, progDbgColor);
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
        bgfx::submit(RdrViewID::GAME, progDbgColor);
    }

    static i64 timeOffset = bx::getHPCounter();
    f32 time = (f32)((bx::getHPCounter()-timeOffset)/f64(bx::getHPFrequency()));


    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        );

    bgfx::setVertexBuffer(0, vbhScreenQuad, 0, 6);

    bgfx::setTexture(0, u_sdiffuse, bgfx::getTexture(fbhGame, 0));
    bgfx::submit(RdrViewID::COMBINE, progGameFinal);

    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA)
        );

    bgfx::setVertexBuffer(0, vbhScreenQuad, 0, 6);

    bgfx::setTexture(0, u_sdiffuse, bgfx::getTexture(fbhUI, 0));
    bgfx::submit(RdrViewID::COMBINE, progUiFinal);

    bgfx::frame();
}

void Renderer::drawMesh(MeshHandle hmesh, const mat4& mtxModel, const vec4& color)
{
    bgfx::setUniform(u_color, color);

    meshSubmit(hmesh, RdrViewID::GAME, progVertShadingColor,
               mtxModel, BGFX_STATE_MASK);
}

void Renderer::drawCubeInstances(const InstanceData* instData, const i32 cubeCount)
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

        bgfx::submit(RdrViewID::GAME, progVertShadingColorInstance);
    }
}