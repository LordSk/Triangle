#include "renderer.h"
#include "dbg_draw.h"
#include "utils.h"
#include <bgfx/bgfx.h>
#include <bx/timer.h>
#include <imgui/imgui.h>

// TODO: emit materials (non-lit)
// TODO: glow

// TODO: do not use this
static bx::FileReader g_fileReader;
static bx::FileWriter g_fileWriter;

bgfx::VertexDecl PosColorNormalVertex::ms_decl;
bgfx::VertexDecl PosUvVertex::decl;

static const PosUvVertex s_wholeScreenQuadVertData[6] = {
    { -1.0f, -1.0f, 0, 0, 1 },
    {  1.0f, -1.0f, 0, 1, 1 },
    {  1.0f,  1.0f, 0, 1, 0 },

    { -1.0f, -1.0f, 0, 0, 1 },
    {  1.0f,  1.0f, 0, 1, 0 },
    { -1.0f,  1.0f, 0, 0, 0 },
};

static PosColorNormalVertex s_originVertData[] =
{
    {0, 0, 0, 0xffff0000 },
    {10, 0, 0, 0xffff0000 },

    {0, 0, 0, 0xff00ff00 },
    {0, 10, 0, 0xff00ff00 },

    {0, 0, 0, 0xff0000ff },
    {0, 0, 10, 0xff0000ff },
};

static PosColorNormalVertex s_cubeRainbowVertData[] =
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

static PosColorNormalVertex s_gridLinesVertData[400];

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
    const bgfx::Caps& caps = *bgfx::getCaps();
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

    if(caps.limits.maxFBAttachments < 3) {
        LOG("ERROR> caps.limits.maxFBAttachments < 3");
        return false;
    }
    LOG("Renderer> caps.limits.maxFBAttachments = %d", caps.limits.maxFBAttachments);


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

    // G buffer textures

    // albedo
    texGbuff_albedo = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::BGRA8,
                                         samplerFlags);
    // emit
    texGbuff_emit = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::BGRA8,
                                         samplerFlags);
    // position
    texGbuff_position = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::RGBA32F,
                                         samplerFlags);
    // normal
    texGbuff_normal = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::BGRA8,
                                         samplerFlags);
    // depth
    texGbuff_depth = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::D24,
                                         samplerFlags);

    bgfx::TextureHandle fbTexGame[5] = {
        texGbuff_albedo,
        texGbuff_emit,
        texGbuff_position,
        texGbuff_normal,
        texGbuff_depth,
    };

    fbhGbuffer = bgfx::createFrameBuffer(arr_count(fbTexGame), fbTexGame, true);

    // light frame buffer
    fbTexLight = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                       1, bgfx::TextureFormat::RGBA16F,
                                       samplerFlags);
    fbhLight = bgfx::createFrameBuffer(1, &fbTexLight, true);


    bgfx::TextureHandle combineFbTextures[] = {
        bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
            1, bgfx::TextureFormat::RGBA16F, samplerFlags),
    };

    fbTexCombine = combineFbTextures[0];

    fbhCombine = bgfx::createFrameBuffer(1, combineFbTextures, true);


    if(!bgfx::isValid(fbhGbuffer)) {
        LOG("ERROR> could not create 'game' frame buffer");
        return false;
    }

    if(!bgfx::isValid(fbhLight)) {
        LOG("ERROR> could not create 'light_pass' frame buffer");
        return false;
    }

    if(!bgfx::isValid(fbhLight)) {
        LOG("ERROR> could not create 'combine' frame buffer");
        return false;
    }

    // Set view clear state.
    bgfx::setViewClear(ViewID::GAME, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0.f);
    bgfx::setViewClear(ViewID::LIGHT, BGFX_CLEAR_COLOR, 0x000000ff, 1.0f, 0.f);
    bgfx::setViewClear(ViewID::UI, BGFX_CLEAR_NONE, 0x0, 1.f, 0.f);
    bgfx::setViewClear(ViewID::COMBINE, BGFX_CLEAR_COLOR, 0x1c1c1cff, 1.0f, 0.f);
    bgfx::setViewClear(ViewID::POST_PROCESS, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, 0x1c1c1cff, 1.0f, 0.f);
    bgfx::setViewClear(ViewID::DBG_DRAW, BGFX_CLEAR_NONE, 0x0, 1.0f, 0.f);
    // UI, DBG_DRAW is on top of everything, dont clear

    bgfx::setViewFrameBuffer(ViewID::GAME, fbhGbuffer);
    bgfx::setViewFrameBuffer(ViewID::LIGHT, fbhLight);
    bgfx::setViewFrameBuffer(ViewID::COMBINE, fbhCombine);

    bgfx::setViewName(ViewID::GAME, "Game");
    bgfx::setViewName(ViewID::LIGHT, "Light");
    bgfx::setViewName(ViewID::UI, "ImGui");
    bgfx::setViewName(ViewID::COMBINE, "Combine");
    bgfx::setViewName(ViewID::POST_PROCESS, "Post processing");
    bgfx::setViewName(ViewID::DBG_DRAW, "Debug draw");

    // Create vertex stream declaration.
    PosColorNormalVertex::init();
    PosUvVertex::init();

    // Create static vertex buffer.
    cubeVbh = bgfx::createVertexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_cubeRainbowVertData, sizeof(s_cubeRainbowVertData)),
            PosColorNormalVertex::ms_decl
            );
    originVbh = bgfx::createVertexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_originVertData, sizeof(s_originVertData)),
            PosColorNormalVertex::ms_decl
            );

    vbhScreenQuad = bgfx::createVertexBuffer(
                        bgfx::makeRef(s_wholeScreenQuadVertData, sizeof(s_wholeScreenQuadVertData)),
                        PosUvVertex::decl
                        );

    u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
    s_albedo = bgfx::createUniform("s_albedo", bgfx::UniformType::Int1); // sampler2D
    s_position = bgfx::createUniform("s_position", bgfx::UniformType::Int1); // sampler2D
    s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Int1); // sampler2D
    s_depth = bgfx::createUniform("s_depth", bgfx::UniformType::Int1); // sampler2D
    u_depthScaleOffset = bgfx::createUniform("u_depthScaleOffset",  bgfx::UniformType::Vec4);
    s_shadowMap = bgfx::createUniform("s_shadowMap",  bgfx::UniformType::Int1);
    s_lightMap = bgfx::createUniform("s_lightMap",  bgfx::UniformType::Int1);
    s_combine = bgfx::createUniform("s_combine",  bgfx::UniformType::Int1);
    s_emit = bgfx::createUniform("s_emit",  bgfx::UniformType::Int1);
    u_lightPos = bgfx::createUniform("u_lightPos",  bgfx::UniformType::Vec4);
    u_lightMtx = bgfx::createUniform("u_lightMtx",  bgfx::UniformType::Mat4);
    u_lightDir = bgfx::createUniform("u_lightDir",  bgfx::UniformType::Vec4);
    u_lightColor1 = bgfx::createUniform("u_lightColor1",  bgfx::UniformType::Vec4);
    u_lightColor2 = bgfx::createUniform("u_lightColor2",  bgfx::UniformType::Vec4);
    u_lightParams = bgfx::createUniform("u_lightParams", bgfx::UniformType::Vec4);
    u_exposure = bgfx::createUniform("u_exposure",  bgfx::UniformType::Vec4);

    progTest = loadProgram(&g_fileReader, "vs_test", "fs_test");
    progVertShading = loadProgram(&g_fileReader, "vs_vertex_shading", "fs_vertex_shading");
    progVertShadingColor = loadProgram(&g_fileReader, "vs_vertex_shading", "fs_vertex_shading_color");
    progVertShadingColorInstance = loadProgram(&g_fileReader, "vs_vertex_shading_instance",
                                                  "fs_vertex_shading_instance");
    progDbgColor = loadProgram(&g_fileReader, "vs_dbg_color", "fs_dbg_color");
    progGameFinal = loadProgram(&g_fileReader, "vs_game_final", "fs_game_final");
    progUiFinal = loadProgram(&g_fileReader, "vs_game_final", "fs_ui_final");
    progGbuffer = loadProgram(&g_fileReader, "vs_gbuffer", "fs_gbuffer");
    progGbufferInst = loadProgram(&g_fileReader, "vs_gbuffer_inst", "fs_gbuffer");
    progLightPoint = loadProgram(&g_fileReader, "vs_light_quad", "fs_light_quad");
    progToneMap = loadProgram(&g_fileReader, "vs_game_final", "fs_tonemap");
    progDirShadowMap = loadProgram(&g_fileReader, "vs_game_final", "fs_shadowmap_quad");


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
            PosColorNormalVertex::ms_decl
            );

    caps_shadowSamplerSupported = (caps.supported & BGFX_CAPS_TEXTURE_COMPARE_LEQUAL) != 0;

    const i32 shadowMapSize = 4096;

    if(caps_shadowSamplerSupported) {
        progShadow = loadProgram(&g_fileReader, "vs_shadow_leq", "fs_shadow_leq");
        progShadowInstance = loadProgram(&g_fileReader, "vs_shadow_leq_instance", "fs_shadow_leq");
        progMeshShadowed = loadProgram(&g_fileReader, "vs_mesh_shadowed", "fs_mesh_shadowed");
        progMeshShadowedInstance = loadProgram(&g_fileReader, "vs_mesh_shadowed_instance",
                                               "fs_mesh_shadowed");

        for(i32 i = 0; i < SHADOW_MAP_COUNT_MAX; i++) {
            texShadowMap[i] = bgfx::createTexture2D(shadowMapSize, shadowMapSize, false,
                                                    1, bgfx::TextureFormat::D24,
                                                    BGFX_TEXTURE_RT|BGFX_TEXTURE_COMPARE_LEQUAL);
        }

        for(i32 i = 0; i < SHADOW_MAP_COUNT_MAX; i++) {
            fbhShadowMap[i] = bgfx::createFrameBuffer(1, &texShadowMap[i], true);

            if(!bgfx::isValid(fbhShadowMap[i])) {
                LOG("ERROR> could not create shadow frame buffer");
                return false;
            }
        }

        for(i32 i = 0; i < SHADOW_MAP_COUNT_MAX; i++) {
            bgfx::setViewRect(ViewID::SHADOW_MAP0+i, 0, 0, shadowMapSize, shadowMapSize);
            bgfx::setViewFrameBuffer(ViewID::SHADOW_MAP0+i, fbhShadowMap[i]);

            char nameStr[32];
            sprintf(nameStr, "ShadowMap%d", i);
            bgfx::setViewName(ViewID::SHADOW_MAP0+i, nameStr);
            bgfx::setViewClear(ViewID::SHADOW_MAP0+i, BGFX_CLEAR_DEPTH,
                               0x303030ff, 1.0f, 0);
        }
    }
    else {
        LOG("Renderer> ERROR shadow samplers are not supported.");
        assert(0);

        /*progShadow = loadProgram(&g_fileReader, "vs_shadow", "fs_shadow");
        progShadowInstance = loadProgram(&g_fileReader, "vs_shadow_instance", "fs_shadow");

        bgfx::TextureHandle fbtextures[] =
        {
            bgfx::createTexture2D(shadowMapSize, shadowMapSize,
                false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT),

            bgfx::createTexture2D(shadowMapSize, shadowMapSize,
                false, 1, bgfx::TextureFormat::D16, BGFX_TEXTURE_RT_WRITE_ONLY),
        };

        texShadowMap = fbtextures[0];
        fbhShadowMap = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);*/
    }

    if(!bgfx::isValid(s_shadowMap) ||
       !bgfx::isValid(u_lightPos) ||
       !bgfx::isValid(u_lightMtx) ||
       !bgfx::isValid(u_lightDir)) {
        LOG("ERROR> could not find uniforms");
        return false;
    }

    f32 depthScaleOffset[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
    if(caps.homogeneousDepth) {
        depthScaleOffset[0] = 0.5f;
        depthScaleOffset[1] = 0.5f;
    }
    bgfx::setUniform(u_depthScaleOffset, depthScaleOffset);

    lightPointList.reserve(256);
    lightDirectionalList.reserve(SHADOW_MAP_COUNT_MAX);

    u8* emitImgData = loadImageRGBA8("../assets/emit_test.png");
    if(!emitImgData) {
        return false;
    }

    const bgfx::Memory* mem = bgfx::makeRef(emitImgData, 1024*1024*4);

    texEmit = bgfx::createTexture2D(1024, 1024,
                                    false,
                                    1, bgfx::TextureFormat::RGBA8,
                                    0
                                    | BGFX_TEXTURE_MIN_POINT
                                    | BGFX_TEXTURE_MAG_POINT
                                    | BGFX_TEXTURE_MIP_POINT
                                    | BGFX_TEXTURE_U_CLAMP
                                    | BGFX_TEXTURE_V_CLAMP,
                                    mem);
    //freeImage(emitImgData);

    // none emit texture
    static u8 dataZero[1024] = {0};
    const bgfx::Memory* memZero = bgfx::makeRef(dataZero, sizeof(dataZero));

    texEmitNone = bgfx::createTexture2D(16, 16,
                                        false, 1, bgfx::TextureFormat::RGBA8, 0
                                        | BGFX_TEXTURE_MIN_POINT
                                        | BGFX_TEXTURE_MAG_POINT
                                        | BGFX_TEXTURE_MIP_POINT
                                        | BGFX_TEXTURE_U_CLAMP
                                        | BGFX_TEXTURE_V_CLAMP,
                                        memZero);

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
    bgfx::destroy(progMeshShadowed);
    bgfx::destroy(progMeshShadowedInstance);
    bgfx::destroy(progGbuffer);
    bgfx::destroy(progGbufferInst);
    bgfx::destroy(progLightPoint);
    bgfx::destroy(progDirShadowMap);
    bgfx::destroy(progToneMap);

    bgfx::destroy(u_color);
    bgfx::destroy(s_albedo);
    bgfx::destroy(s_position);
    bgfx::destroy(s_normal);
    bgfx::destroy(s_depth);
    bgfx::destroy(s_emit);
    bgfx::destroy(u_depthScaleOffset);
    bgfx::destroy(s_shadowMap);
    bgfx::destroy(s_lightMap);
    bgfx::destroy(s_combine);
    bgfx::destroy(u_lightPos);
    bgfx::destroy(u_lightMtx);
    bgfx::destroy(u_lightDir);
    bgfx::destroy(u_lightColor1);
    bgfx::destroy(u_lightColor2);
    bgfx::destroy(u_lightParams);
    bgfx::destroy(u_exposure);

    bgfx::destroy(fbhGbuffer);
    bgfx::destroy(fbhLight);
    bgfx::destroy(fbhCombine);

    for(i32 i = 0; i < SHADOW_MAP_COUNT_MAX; i++) {
         bgfx::destroy(fbhShadowMap[i]);
    }

    bgfx::shutdown();
}

void Renderer::setView(const mat4& proj, const mat4& view)
{
    mtxProj = proj;
    mtxView = view;
}

void Renderer::fitShadowMapToSceneBounds(ShadowMapDirectional* light)
{
    const vec3 orthoMin = vec3Splat(-1.0f);
    const vec3 orthoMax = vec3Splat(1.0f);
    const vec3 bmin = light->worldArea.bmin;
    const vec3 bmax = light->worldArea.bmax;

    mat4 mtxLightProj, mtxLightView;
    bx::mtxLookAtRh(mtxLightView, light->pos, light->pos + light->dir, vec3{0, 0, 1});
    bx::mtxOrthoRh(mtxLightProj, orthoMin.x, orthoMax.x, orthoMin.y, orthoMax.y,
                   orthoMin.z, orthoMax.z, 0.0f, true);

    mat4 mtxLightViewProj;
    bx::mtxMul(mtxLightViewProj, mtxLightView, mtxLightProj);

    // bound points
    const vec3 points[8] = {
        bmin,
        {bmax.x, bmin.y, bmin.z},
        {bmin.x, bmax.y, bmin.z},
        {bmax.x, bmax.y, bmin.z},

        {bmin.x, bmin.y, bmax.z},
        {bmax.x, bmin.y, bmax.z},
        {bmin.x, bmax.y, bmax.z},
        bmax,
    };

    f32 left = FLT_MAX;
    f32 right = -FLT_MAX;
    f32 top = FLT_MAX;
    f32 bottom = -FLT_MAX;
    f32 near = FLT_MAX;
    f32 far = -FLT_MAX;

    for(i32 i = 0; i < 8; i++) {
        vec3 lsp; // light space point
        bx::vec3MulMtx(lsp, points[i], mtxLightViewProj);

        left = mmin(lsp.x, left);
        right = mmax(lsp.x, right);
        top = mmin(lsp.y, top);
        bottom = mmax(lsp.y, bottom);
        near = mmin(lsp.z, near);
        far = mmax(lsp.z, far);
    }

    light->orthoMin.x = left;
    light->orthoMin.y = top;
    light->orthoMin.z = near;
    light->orthoMax.x = right;
    light->orthoMax.y = bottom;
    light->orthoMax.z = far;
}

void Renderer::setupDirectionalShadowMap(const ShadowMapDirectional& light, const i32 shadowMapId)
{
    mat4 mtxLightProj, mtxLightView;
    bx::mtxLookAtRh(mtxLightView, light.pos, light.pos + light.dir, vec3{0, 0, 1});

    const bgfx::Caps* caps = bgfx::getCaps();
    bx::mtxOrthoRh(mtxLightProj, light.orthoMin.x, light.orthoMax.x, light.orthoMax.y, light.orthoMin.y,
                   light.orthoMin.z, light.orthoMax.z, 0.0f, caps->homogeneousDepth);

    bgfx::setViewTransform(ViewID::SHADOW_MAP0 + shadowMapId, mtxLightView, mtxLightProj);

    const float sy = caps->originBottomLeft ? 0.5f : -0.5f;
    const float sz = caps->homogeneousDepth ? 0.5f :  1.0f;
    const float tz = caps->homogeneousDepth ? 0.5f :  0.0f;
    const mat4 mtxCrop =
    {
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f,   sy, 0.0f, 0.0f,
        0.0f, 0.0f, sz,   0.0f,
        0.5f, 0.5f, tz,   1.0f,
    };

    mat4 mtxTmp;
    bx::mtxMul(mtxTmp,   mtxLightProj, mtxCrop);
    bx::mtxMul(mtxLightDirectional[shadowMapId], mtxLightView, mtxTmp);
}

void Renderer::lightpass()
{
    const i32 lightPointCount = lightPointList.count();
    const LightPoint* lightPoints = lightPointList.data();

    mat4 mtxViewProj;
    bx::mtxMul(mtxViewProj, mtxView, mtxProj);

    for(i32 i = 0; i < lightPointCount; i++) {
        const LightPoint& lp = lightPoints[i];
        /*const f32 constant = 1.0f;
        const f32 linear = lp.radius;
        const f32 quadratic = lp.slope;
        const f32 lightMax = mmax(mmax(lp.color1.x, lp.color1.y), lp.color1.z) *
                             lp.intensity;
        const f32 radius = (-linear +  sqrtf(linear * linear - 4 * quadratic *
                                             (constant - (256.0 / 1.0) * lightMax))) / (2 * quadratic);*/
        const f32 radius = lp.radius;
        const AABB aabb = {lp.pos - vec3Splat(radius), lp.pos + vec3Splat(radius)};
        const AABB clipSpaceAabb = aabbTransformSpace(aabb, mtxViewProj);

        if(dbgLightBoundingBox) {
            Transform tf;
            tf.pos = aabb.bmin;
            tf.scale = aabb.bmax - aabb.bmin;
            dbgDrawRectLine(tf, vec4FromVec3(lp.color1, 1.0));
        }

        const vec3 c0 = clipSpaceAabb.bmin;
        const vec3 c1 = clipSpaceAabb.bmax;
        const vec2 uv0 = (vec2{c0.x, -c0.y} + vec2Splat(1.0f)) * 0.5f;
        const vec2 uv1 = (vec2{c1.x, -c1.y} + vec2Splat(1.0f)) * 0.5f;

        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;
        if(bgfx::allocTransientBuffers(&tvb, PosUvVertex::decl, 4, &tib, 6)) {
            PosUvVertex* vertex = (PosUvVertex*)tvb.data;
            vertex->x = c0.x;
            vertex->y = c0.y;
            vertex->z = 0.0f;
            vertex->u = uv0.x;
            vertex->v = uv0.y;
            ++vertex;

            vertex->x = c1.x;
            vertex->y = c0.y;
            vertex->z = 0.0f;
            vertex->u = uv1.x;
            vertex->v = uv0.y;
            ++vertex;

            vertex->x = c0.x;
            vertex->y = c1.y;
            vertex->z = 0.0f;
            vertex->u = uv0.x;
            vertex->v = uv1.y;
            ++vertex;

            vertex->x = c1.x;
            vertex->y = c1.y;
            vertex->z = 0.0f;
            vertex->u = uv1.x;
            vertex->v = uv1.y;
            ++vertex;

            uint16_t* indices = (uint16_t*)tib.data;
            *indices++ = 0;
            *indices++ = 1;
            *indices++ = 3;
            *indices++ = 0;
            *indices++ = 3;
            *indices++ = 2;

            bgfx::setTexture(1, s_position, texGbuff_position);
            bgfx::setTexture(2, s_normal, texGbuff_normal);
            const vec4 lightPos4 = vec4FromVec3(lp.pos, 1.0);
            bgfx::setUniform(u_lightPos, lightPos4);
            bgfx::setUniform(u_lightColor1, lp.color1);
            bgfx::setUniform(u_lightColor2, lp.color2);
            const vec4 lightParams = {lp.intensity, lp.radius, lp.slope, lp.falloff};
            bgfx::setUniform(u_lightParams, lightParams);

            bgfx::setVertexBuffer(0, &tvb);
            bgfx::setIndexBuffer(&tib);

            bgfx::setState(0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_BLEND_ADD
                );
            bgfx::submit(ViewID::LIGHT, progLightPoint);
        }
    }

    const i32 lightDirectionalCount = lightDirectionalList.count();
    const LightDirectional* lightDirectionals = lightDirectionalList.data();
    assert(lightDirectionalCount <= SHADOW_MAP_COUNT_MAX); // TODO: decouple from shadow maps

    for(i32 i = 0; i < lightDirectionalCount; i++) {
        const LightDirectional& ld = lightDirectionals[i];
        ShadowMapDirectional smd;
        smd.pos = ld.pos;
        smd.dir = ld.dir;
        smd.worldArea = ld.worldArea;

        fitShadowMapToSceneBounds(&smd);
        setupDirectionalShadowMap(smd, i);

        if(dbgLightBoundingBox) {
            const vec3 dir = vec3Norm(smd.dir);
            const vec4 lightColor = vec4FromVec3(ld.color, 1.0);
            dbgDrawSphere(smd.pos, 1, lightColor);
            dbgDrawLine(smd.pos, smd.pos + dir * 5.f, lightColor, 0.1f);

            dbgDrawOrthoFrustrum(smd.pos, smd.pos + smd.dir, smd.orthoMin.z,
                                 smd.orthoMax.z,
                                 smd.orthoMin.x, smd.orthoMax.x, smd.orthoMin.y,
                                 smd.orthoMax.y,
                                 vec4FromVec3(ld.color, 1.0), 0.1f);

            Transform lightWorldAreaTf;
            lightWorldAreaTf.pos = smd.worldArea.bmin;
            lightWorldAreaTf.scale = smd.worldArea.bmax - smd.worldArea.bmin;
            dbgDrawRectLine(lightWorldAreaTf, lightColor);
        }
    }

    for(i32 i = 0; i < lightDirectionalCount; i++) {
        const LightDirectional& ld = lightDirectionals[i];

        // TODO: clip this based on shadow map area
        bgfx::setState(0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_BLEND_ADD
        );

        bgfx::setVertexBuffer(0, vbhScreenQuad, 0, 6);

        bgfx::setUniform(u_lightMtx, mtxLightDirectional[i]);
        bgfx::setUniform(u_lightDir, ld.dir);
        bgfx::setUniform(u_lightPos, ld.pos);
        bgfx::setUniform(u_lightColor1, ld.color);
        vec4 lightParams = {ld.intensity, 0.0f, 0.0f, 0.0f};
        bgfx::setUniform(u_lightParams, lightParams);

        bgfx::setTexture(1, s_position, texGbuff_position);
        bgfx::setTexture(2, s_normal, texGbuff_normal);
        bgfx::setTexture(4, s_shadowMap, texShadowMap[i]);
        bgfx::submit(ViewID::LIGHT, progDirShadowMap);
    }
}

void Renderer::dbgDoMenuBar()
{
    ImGui::SliderFloat("exposure", &dbgExposure, 0.1f, 10.0f, "%.3f");
}

void Renderer::frame()
{
    bgfx::setViewRect(ViewID::GAME, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(ViewID::LIGHT, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(ViewID::UI, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(ViewID::COMBINE, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(ViewID::POST_PROCESS, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(ViewID::DBG_DRAW, 0, 0, u16(renderWidth), u16(renderHeight));

    bgfx::setViewTransform(ViewID::GAME, mtxView, mtxProj);
    bgfx::setViewTransform(ViewID::DBG_DRAW, mtxView, mtxProj);

    // This dummy draw call is here to make sure that view x is cleared
    // if no other draw calls are submitted to view x.
    bgfx::touch(ViewID::GAME);
    bgfx::touch(ViewID::LIGHT);
    bgfx::touch(ViewID::UI);
    bgfx::touch(ViewID::COMBINE);
    bgfx::touch(ViewID::POST_PROCESS);
    bgfx::touch(ViewID::DBG_DRAW);

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
        bgfx::submit(ViewID::DBG_DRAW, progDbgColor);
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

        const f32 white[] = {0.6f, 0.6f, 0.6f, 1};
        bgfx::setUniform(u_color, white);
        bgfx::setVertexBuffer(0, gridVbh, 0, BX_COUNTOF(s_gridLinesVertData));
        bgfx::submit(ViewID::DBG_DRAW, progDbgColor);
    }

    // light pass
    lightpass();

    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_CULL_CW
        | BGFX_STATE_BLEND_ALPHA
        );

    bgfx::setVertexBuffer(0, vbhScreenQuad, 0, 6);

    bgfx::setTexture(0, s_albedo, texGbuff_albedo);
    bgfx::setTexture(1, s_emit, texGbuff_emit);
    bgfx::setTexture(5, s_lightMap, fbTexLight);
    bgfx::submit(ViewID::COMBINE, progGameFinal);

    // tone mapping
    bgfx::setTexture(0, s_combine, fbTexCombine);
    bgfx::setTexture(1, s_depth, texGbuff_depth);
    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CW
        );

    vec4 exposure = {dbgExposure};
    bgfx::setUniform(u_exposure, exposure);

    bgfx::setVertexBuffer(0, vbhScreenQuad, 0, 6);
    bgfx::submit(ViewID::POST_PROCESS, progToneMap);

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}

void Renderer::drawMesh(MeshHandle hmesh, const mat4& mtxModel, const vec4& color, bool unlit)
{
    bgfx::setUniform(u_color, color);

    // TODO: pass in emit texture and emit intensity (uniform)
    bgfx::setTexture(0, s_emit, unlit ? texEmit : texEmitNone);

    meshSubmit(hmesh, ViewID::GAME, progGbuffer,
               mtxModel, BGFX_STATE_MASK);

    u64 state = 0
        | (caps_shadowSamplerSupported ? 0 : BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A)
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CCW
        /*| BGFX_STATE_MSAA*/;

    // draw on shadow maps
    // TODO: cull outside of light worldArea bounds
    const i32 lightDirectionalCount = lightDirectionalList.count();
    const LightDirectional* lightDirectionals = lightDirectionalList.data();

    for(i32 i = 0; i < lightDirectionalCount; i++) {
        const LightDirectional& ld = lightDirectionals[i];
        bgfx::setUniform(u_lightPos, ld.pos);
        bgfx::setUniform(u_lightDir, ld.dir);
        bgfx::setUniform(u_lightMtx, mtxLightDirectional[i]);
        meshSubmit(hmesh, ViewID::SHADOW_MAP0 + i, progShadow,
                   mtxModel, state);
    }
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

        bgfx::setTexture(0, s_emit, texEmitNone);
        bgfx::setVertexBuffer(0, cubeVbh, 0, 36);
        bgfx::setInstanceDataBuffer(&idb);

        bgfx::setState(0
            | BGFX_STATE_WRITE_MASK
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CCW
            | BGFX_STATE_MSAA
            );

        bgfx::submit(ViewID::GAME, progGbufferInst);

        if(dropShadow) {
            // draw on shadow maps
            // TODO: cull outside of light worldArea bounds
            const i32 lightDirectionalCount = lightDirectionalList.count();
            const LightDirectional* lightDirectionals = lightDirectionalList.data();

            for(i32 i = 0; i < lightDirectionalCount; i++) {
                const LightDirectional& ld = lightDirectionals[i];
                bgfx::setUniform(u_lightPos, ld.pos);
                bgfx::setUniform(u_lightDir, ld.dir);
                bgfx::setUniform(u_lightMtx, mtxLightDirectional[i]);


                bgfx::setVertexBuffer(0, cubeVbh, 0, 36);
                bgfx::setInstanceDataBuffer(&idb);

                bgfx::setState(0
                   | (caps_shadowSamplerSupported ? 0 : BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A)
                   | BGFX_STATE_WRITE_Z
                   | BGFX_STATE_DEPTH_TEST_LESS
                   | BGFX_STATE_CULL_CW
                   /*| BGFX_STATE_MSAA*/);

                bgfx::submit(ViewID::SHADOW_MAP0 + i, progShadowInstance);
            }
        }
    }
}

void Renderer::drawCube(mat4 mtxModel, vec4 color)
{
    bgfx::setTransform(mtxModel);
    bgfx::setUniform(u_color, color);
    bgfx::setTexture(0, s_emit, texEmitNone);

    bgfx::setVertexBuffer(0, cubeVbh, 0, 36);
    bgfx::setState(0
        | BGFX_STATE_WRITE_MASK
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CCW
        | BGFX_STATE_MSAA
        );

    bgfx::submit(ViewID::GAME, progGbuffer);


    // draw on shadow maps
    // TODO: cull outside of light worldArea bounds
    const i32 lightDirectionalCount = lightDirectionalList.count();
    const LightDirectional* lightDirectionals = lightDirectionalList.data();

    for(i32 i = 0; i < lightDirectionalCount; i++) {
        const LightDirectional& ld = lightDirectionals[i];
        bgfx::setUniform(u_lightPos, ld.pos);
        bgfx::setUniform(u_lightDir, ld.dir);
        bgfx::setUniform(u_lightMtx, mtxLightDirectional[i]);
        bgfx::setTransform(mtxModel);

        bgfx::setVertexBuffer(0, cubeVbh, 0, 36);
        bgfx::setState(0
           | (caps_shadowSamplerSupported ? 0 : BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A)
           | BGFX_STATE_WRITE_Z
           | BGFX_STATE_DEPTH_TEST_LESS
           | BGFX_STATE_CULL_CCW
           | BGFX_STATE_MSAA);

        bgfx::submit(ViewID::SHADOW_MAP0 + i, progShadow);
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
