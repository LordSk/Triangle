#include "renderer.h"
#include "dbg_draw.h"
#include "utils.h"
#include <bgfx/bgfx.h>
#include <bx/timer.h>
#include <imgui/imgui.h>

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
    bgfx::TextureHandle fbTexGame[4];
    // albedo
    fbTexGame[0] = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::BGRA8,
                                         samplerFlags);
    // position
    fbTexGame[1] = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::RGBA32F,
                                         samplerFlags);
    // normal
    fbTexGame[2] = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::BGRA8,
                                         samplerFlags);
    // depth
    fbTexGame[3] = bgfx::createTexture2D(u16(renderWidth), u16(renderHeight), false,
                                         1, bgfx::TextureFormat::D24,
                                         samplerFlags);
    fbhGame = bgfx::createFrameBuffer(arr_count(fbTexGame), fbTexGame, true);

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
    bgfx::setViewClear(ViewID::GAME, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0.f);
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
    s_albedo = bgfx::createUniform("s_albedo", bgfx::UniformType::Int1); // sampler2D
    s_position = bgfx::createUniform("s_position", bgfx::UniformType::Int1); // sampler2D
    s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Int1); // sampler2D
    s_depth = bgfx::createUniform("s_depth", bgfx::UniformType::Int1); // sampler2D

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

    u_depthScaleOffset = bgfx::createUniform("u_depthScaleOffset",  bgfx::UniformType::Vec4);
    s_shadowMap = bgfx::createUniform("s_shadowMap",  bgfx::UniformType::Int1);
    u_lightPos = bgfx::createUniform("u_lightPos",  bgfx::UniformType::Vec4);
    u_lightMtx = bgfx::createUniform("u_lightMtx",  bgfx::UniformType::Mat4);
    u_lightDir = bgfx::createUniform("u_lightDir",  bgfx::UniformType::Vec4);

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
    bgfx::destroy(s_albedo);
    bgfx::destroy(u_depthScaleOffset);

    bgfx::destroy(fbhGame);
    bgfx::destroy(fbhUI);

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

    vec4 lightPos;
    memmove(lightPos.data, light.pos.data, sizeof(light.pos));
    bgfx::setUniform(u_lightPos, lightPos);

    vec3 dir = vec3Norm(light.dir);
    vec4 lightDir;
    memmove(lightDir.data, dir.data, sizeof(dir));
    bgfx::setUniform(u_lightDir, lightDir);

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
    bx::mtxMul(mtxLight0, mtxLightView, mtxTmp);

    bgfx::setUniform(u_lightMtx, mtxLight0);
}

void Renderer::frame()
{
    bgfx::setViewRect(ViewID::GAME, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(ViewID::UI, 0, 0, u16(renderWidth), u16(renderHeight));
    bgfx::setViewRect(ViewID::COMBINE, 0, 0, u16(renderWidth), u16(renderHeight));

    bgfx::setViewTransform(ViewID::GAME, mtxView, mtxProj);

    // shadow map
    static i64 t0 = bx::getHPCounter();
    i64 now = bx::getHPCounter();
    const f64 freq = f64(bx::getHPFrequency());
    f32 time = f32((now - t0)/ freq) * 0.5;

    Camera camLight;
    camLight.eye = {50, 25, 30};
    camLight.at = {50 + cosf(time) * 30.0f, 25.001f + sinf(time) * 30.0f, 0};
    //camLight.at = {50 + 15, 25.001f, 0};

    ShadowMapDirectional sunlight;
    sunlight.pos = camLight.eye;
    sunlight.dir = camLight.at - camLight.eye;
    sunlight.worldArea = {
        {-10, -10, -10},
        {120, 65, 30}
    };

    fitShadowMapToSceneBounds(&sunlight);
    setupDirectionalShadowMap(sunlight, 0);

#if 0
    const vec3 ld = vec3Norm(sunlight.dir);
    dbgDrawSphere(sunlight.pos, 1, vec4Splat(1.0));
    dbgDrawLine(sunlight.pos, sunlight.pos + ld * 5.f, vec4Splat(1.0), 0.1f);

    dbgDrawOrthoFrustrum(sunlight.pos, sunlight.pos + sunlight.dir, sunlight.orthoMin.z, sunlight.orthoMax.z,
                         sunlight.orthoMin.x, sunlight.orthoMax.x, sunlight.orthoMin.y, sunlight.orthoMax.y,
                         vec4{1, 1, 0, 1}, 0.1f);

    Transform lightWorldAreaTf;
    lightWorldAreaTf.pos = sunlight.worldArea.bmin;
    lightWorldAreaTf.scale = sunlight.worldArea.bmax - sunlight.worldArea.bmin;
    dbgDrawRectLine(lightWorldAreaTf, vec4{0, 1, 0, 1});
#endif


    // This dummy draw call is here to make sure that view x is cleared
    // if no other draw calls are submitted to view x.
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


    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_BLEND_ALPHA
        );

    bgfx::setVertexBuffer(0, vbhScreenQuad, 0, 6);

    bgfx::setUniform(u_lightMtx, mtxLight0);
    bgfx::setTexture(0, s_albedo, bgfx::getTexture(fbhGame, 0));
    bgfx::setTexture(1, s_position, bgfx::getTexture(fbhGame, 1));
    bgfx::setTexture(2, s_normal, bgfx::getTexture(fbhGame, 2));
    bgfx::setTexture(3, s_depth, bgfx::getTexture(fbhGame, 3));
    bgfx::setTexture(4, s_shadowMap, texShadowMap[0]);
    bgfx::submit(ViewID::COMBINE, progGameFinal);

    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA)
        );

    bgfx::setVertexBuffer(0, vbhScreenQuad, 0, 6);

    bgfx::setTexture(0, s_albedo, bgfx::getTexture(fbhUI, 0));
    bgfx::submit(ViewID::COMBINE, progUiFinal);

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}

void Renderer::drawMesh(MeshHandle hmesh, const mat4& mtxModel, const vec4& color)
{
    bgfx::setUniform(u_color, color);

    meshSubmit(hmesh, ViewID::GAME, progGbuffer,
               mtxModel, BGFX_STATE_MASK);

    u64 state = 0
        | (caps_shadowSamplerSupported ? 0 : BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A)
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CCW
        /*| BGFX_STATE_MSAA*/;

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

        bgfx::submit(ViewID::GAME, progGbufferInst);

        if(dropShadow) {
            bgfx::setVertexBuffer(0, cubeVbh, 0, 36);
            bgfx::setInstanceDataBuffer(&idb);

            bgfx::setState(0
               | (caps_shadowSamplerSupported ? 0 : BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A)
               | BGFX_STATE_WRITE_Z
               | BGFX_STATE_DEPTH_TEST_LESS
               | BGFX_STATE_CULL_CW
               /*| BGFX_STATE_MSAA*/);

            bgfx::submit(ViewID::SHADOW_MAP0, progShadowInstance);
        }
    }
}

void Renderer::drawCube(mat4 mtxModel, vec4 color)
{
    bgfx::setTransform(mtxModel);
    bgfx::setUniform(u_color, color);

    bgfx::setVertexBuffer(0, cubeVbh, 0, 36);
    bgfx::setState(0
        | BGFX_STATE_WRITE_MASK
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CCW
        | BGFX_STATE_MSAA
        );

    bgfx::submit(ViewID::GAME, progGbuffer);

    bgfx::setTransform(mtxModel);
    bgfx::setUniform(u_color, color);

    bgfx::setVertexBuffer(0, cubeVbh, 0, 36);
    bgfx::setState(0
       | (caps_shadowSamplerSupported ? 0 : BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A)
       | BGFX_STATE_WRITE_Z
       | BGFX_STATE_DEPTH_TEST_LESS
       | BGFX_STATE_CULL_CCW
       | BGFX_STATE_MSAA);

    bgfx::submit(ViewID::SHADOW_MAP0, progShadow);
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
