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

    const i32 shadowMapSize = 4096;

    if(caps_shadowSamplerSupported) {
        progShadow = loadProgram(&g_fileReader, "vs_shadow_leq", "fs_shadow_leq");
        progShadowInstance = loadProgram(&g_fileReader, "vs_shadow_leq_instance", "fs_shadow_leq");
        progMeshShadowed = loadProgram(&g_fileReader, "vs_mesh_shadowed", "fs_mesh_shadowed");
        progMeshShadowedInstance = loadProgram(&g_fileReader, "vs_mesh_shadowed_instance",
                                               "fs_mesh_shadowed");

        bgfx::TextureHandle fbtextures[] =
        {
            bgfx::createTexture2D(shadowMapSize, shadowMapSize, false,
                1, bgfx::TextureFormat::D24, BGFX_TEXTURE_RT|BGFX_TEXTURE_COMPARE_LEQUAL),
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

void Renderer::fitShadowMapToSceneBounds(LightDirectional* light, vec3 bmin, vec3 bmax)
{
    mat4 mtxLightProj, mtxLightView;
    bx::mtxLookAtRh(mtxLightView, light->pos, light->pos + light->dir, vec3{0, 0, 1});

    const bgfx::Caps* caps = bgfx::getCaps();
    bx::mtxOrthoRh(mtxLightProj, light->left, light->right, light->bottom, light->top,
                   light->near_, light->far_, 0.0f, true);

    mat4 mtxLightViewProj;
    bx::mtxMul(mtxLightViewProj, mtxLightView, mtxLightProj);

    // bound points
    vec3 points[8] = {
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

    mat4 mtxInvViewProj;
    mat4 mtxInvProj;
    bx::mtxInverse(mtxInvViewProj, mtxLightViewProj);
    bx::mtxInverse(mtxInvProj, mtxLightProj);

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

    const vec3 lsPoints[8] {
        {left, top, near},
        {right, top, near},
        {left, bottom, near},
        {right, bottom, near},

        {left, top, far},
        {right, top, far},
        {left, bottom, far},
        {right, bottom, far},
    };

    f32 fixedLeft = FLT_MAX;
    f32 fixedRight = -FLT_MAX;
    f32 fixedTop = FLT_MAX;
    f32 fixedBottom = -FLT_MAX;
    f32 fixedNear = FLT_MAX;
    f32 fixedFar = -FLT_MAX;

    for(i32 i = 0; i < 8; i++) {
        vec3 projp;
        bx::vec3MulMtx(projp, lsPoints[i], mtxInvProj);

        fixedLeft = mmin(projp.x, fixedLeft);
        fixedRight = mmax(projp.x, fixedRight);
        fixedTop = mmin(projp.y, fixedTop);
        fixedBottom = mmax(projp.y, fixedBottom);
        fixedNear = mmin(-projp.z, fixedNear); // I don't why z is reversed here
        fixedFar = mmax(-projp.z, fixedFar);
    }

    light->left = fixedLeft;
    light->right = fixedRight;
    light->top = fixedTop;
    light->bottom = fixedBottom;
    light->near_ = fixedNear;
    light->far_ = fixedFar;
}

void Renderer::setupDirectionalShadowMap(const LightDirectional& light)
{
    mat4 mtxLightProj, mtxLightView;
    bx::mtxLookAtRh(mtxLightView, light.pos, light.pos + light.dir, vec3{0, 0, 1});

    // dbg draw light object
    vec3 ld = vec3Norm(light.dir);
    dbgDrawSphere(light.pos, 1, vec4Splat(1.0));
    dbgDrawLine(light.pos, light.pos + ld * 5.f, vec4Splat(1.0), 0.1f);

    const bgfx::Caps* caps = bgfx::getCaps();
    bx::mtxOrthoRh(mtxLightProj, light.left, light.right, light.bottom, light.top,
                   light.near_, light.far_, 0.0f, caps->homogeneousDepth);

    bgfx::setViewTransform(ViewID::SHADOW_MAP0, mtxLightView, mtxLightProj);

    vec4 lightPos;
    memmove(lightPos.data, light.pos.data, sizeof(light.pos));
    bgfx::setUniform(u_lightPos, lightPos);

    vec3 dir = ld;
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
    f32 time = f32((now - t0)/ freq) * 0.2;

    Camera camLight;
    camLight.eye = {50, 25, 30};
    camLight.at = {50 + cosf(time) * 30.0f, 25.001f + sinf(time) * 30.0f, 0};
    //camLight.at = {50 + 15, 25.001f + 15, 0};

    LightDirectional sunlight;
    sunlight.left = -10.f;
    sunlight.right = 10.f;
    sunlight.top = -10.f;
    sunlight.bottom = 10.f;
    sunlight.near_ = -10.f;
    sunlight.far_ = 10.f;
    sunlight.pos = camLight.eye;
    sunlight.dir = camLight.at - camLight.eye;

    // TODO: plug this in
    vec3 sceneBoundPos = {-10, -10, -10};
    vec3 sceneBoundSize = {120, 65, 30};

    fitShadowMapToSceneBounds(&sunlight, sceneBoundPos, sceneBoundPos + sceneBoundSize);
    setupDirectionalShadowMap(sunlight);

    dbgDrawOrthoFrustrum(sunlight.pos, sunlight.pos + sunlight.dir, sunlight.near_, sunlight.far_,
                         sunlight.left, sunlight.right, sunlight.top, sunlight.bottom,
                         vec4{1, 1, 0, 1}, 0.1f);


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
    bgfx::setUniform(u_lightMtx, mtxLight0);
    bgfx::setTexture(0, s_shadowMap, texShadowMap);
    bgfx::setUniform(u_color, color);

    meshSubmit(hmesh, ViewID::GAME, progMeshShadowed,
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

        bgfx::setTexture(0, s_shadowMap, texShadowMap);

        bgfx::submit(ViewID::GAME, progMeshShadowedInstance);

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

    bgfx::setTexture(0, s_shadowMap, texShadowMap);
    bgfx::submit(ViewID::GAME, progMeshShadowed);

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
