#ifdef _WIN32
    #define SDL_MAIN_HANDLED
    #include <windows.h>
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "base.h"
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <bx/file.h>
#include <bx/timer.h>
#include <bx/rng.h>
#include <imgui/bgfx_imgui.h>

#define STATIC_MESH_DATA
#include "static_mesh.h"

#include "mesh_load.h"
#include "player_ship.h"
#include "utils.h"
#include "dbg_draw.h"
#include "collision.h"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

namespace im = ImGui;

// TODO: move this
static bx::FileReader g_fileReader;
static bx::FileWriter g_fileWriter;
static bx::RngMwc g_rng;

bgfx::VertexDecl PosColorVertex::ms_decl;

struct Room
{
    Transform tfRoom;
    vec3 size;
    std::vector<Transform> cubeTransforms;
    std::vector<mat4> cubeTfMtx;
    PhysWorld physWorld;

    void make(vec3 size_, const i32 cubeSize) {
        i32 sizexi = bx::ceil(size_.x);
        i32 sizeyi = bx::ceil(size_.y);
        size.x = sizexi + (cubeSize - (sizexi % cubeSize));
        size.y = sizeyi + (cubeSize - (sizeyi % cubeSize));
        size.z = size_.z;
        i32 width =  size.x / cubeSize;
        i32 height = size.y / cubeSize;

        cubeTransforms.clear();
        cubeTransforms.reserve(width * height + 4);

        // background
        for(i32 y = 0; y < height; y++) {
            for(i32 x = 0; x < width; x++) {
                Transform tfCube;
                tfCube.pos.x = cubeSize * 0.5 + x * cubeSize;
                tfCube.pos.y = cubeSize * 0.5 + y * cubeSize;
                tfCube.pos.z = -size.z + bx::frnd(&g_rng) * 0.5;
                tfCube.scale = {(f32)cubeSize, (f32)cubeSize, 1.f};
                bx::quatIdentity(tfCube.rot);
                cubeTransforms.push_back(tfCube);
            }
        }

        // walls
        Transform tfWall;
        tfWall.scale = {size.x, cubeSize * 0.5f, size.z};
        tfWall.pos.x = size.x * 0.5f;
        tfWall.pos.y = -tfWall.scale.y * 0.5f;
        tfWall.pos.z = -size.z * 0.5;
        cubeTransforms.push_back(tfWall);

        tfWall = {};
        tfWall.scale = {size.x, cubeSize * 0.5f, size.z};
        tfWall.pos.x = size.x * 0.5f;
        tfWall.pos.y = size.y + tfWall.scale.y * 0.5f;
        tfWall.pos.z = -size.z * 0.5;
        cubeTransforms.push_back(tfWall);

        tfWall = {};
        tfWall.scale = {cubeSize * 0.5f, size.y, size.z};
        tfWall.pos.x = -tfWall.scale.x * 0.5f;
        tfWall.pos.y = size.y * 0.5;
        tfWall.pos.z = -size.z * 0.5;
        cubeTransforms.push_back(tfWall);

        tfWall = {};
        tfWall.scale = {cubeSize * 0.5f, size.y, size.z};
        tfWall.pos.x = size.x + tfWall.scale.x * 0.5f;
        tfWall.pos.y = size.y * 0.5;
        tfWall.pos.z = -size.z * 0.5;
        cubeTransforms.push_back(tfWall);

        Collider colWall;
        OrientedBoundingBox obbWall;
        obbWall.origin = vec2{ 0, -cubeSize * 0.5f };
        obbWall.size = vec2{ size.x, cubeSize * 0.5f };
        obbWall.angle = 0;
        physWorld.addStaticCollider(colWall.makeObb(obbWall));

        obbWall.origin = vec2{ 0, size.y };
        obbWall.size = vec2{ size.x, cubeSize * 0.5f };
        physWorld.addStaticCollider(colWall.makeObb(obbWall));

        obbWall.origin = vec2{ -cubeSize * 0.5f, 0 };
        obbWall.size = vec2{ cubeSize * 0.5f, size.y };
        physWorld.addStaticCollider(colWall.makeObb(obbWall));

        obbWall.origin = vec2{ size.x, 0 };
        obbWall.size = vec2{ cubeSize * 0.5f, size.y };
        physWorld.addStaticCollider(colWall.makeObb(obbWall));
    }

    void dbgDraw() {
        physWorld.dbgDraw(tfRoom.pos);
    }
};

struct CameraFreeFlight
{
    vec3 pos = {0, 0, 10};
    vec3 dir = {1, 0, 0};
    vec3 up = {0, 0, 1};
    quat rot;
    f32 speed = 30;

    struct {
        bool8 forward;
        bool8 backward;
        bool8 left;
        bool8 right;
        bool8 up;
        bool8 down;
    } input = {};

    f64 pitch = 0;
    f64 yaw = 0;

    void handleEvent(const SDL_Event& event) {
        if(event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            switch(event.key.keysym.sym) {
                case SDLK_z: input.forward = true; break;
                case SDLK_s: input.backward = true; break;
                case SDLK_q: input.left = true; break;
                case SDLK_d: input.right = true; break;
                case SDLK_a: input.up = true; break;
                case SDLK_e: input.down = true; break;
            }
            return;
        }
        if(event.type == SDL_KEYUP) {
            switch(event.key.keysym.sym) {
                case SDLK_z: input.forward = false; break;
                case SDLK_s: input.backward = false; break;
                case SDLK_q: input.left = false; break;
                case SDLK_d: input.right = false; break;
                case SDLK_a: input.up = false; break;
                case SDLK_e: input.down = false; break;
            }
            return;
        }

        if(event.type == SDL_MOUSEMOTION) {
            // TODO: move this to applyInput
            yaw = 0;
            pitch = 0;
            yaw += event.motion.xrel / (bx::kPi2 * 100.0);
            pitch += event.motion.yrel / (bx::kPi2 * 100.0);

            vec3 right;
            bx::vec3Cross(right, dir, up);
            quat qyaw;
            quat qpitch;
            bx::quatRotateAxis(qyaw, up, yaw);
            bx::quatRotateAxis(qpitch, right, pitch);
            bx::quatMul(rot, qyaw, qpitch);
            bx::quatNorm(rot, rot);
            //dir = {1, 0, 0};
            bx::vec3MulQuat(dir, dir, rot);
            bx::vec3Norm(dir, dir);
            return;
        }
    }

    void applyInput(f64 delta) {
        vec3 right;
        bx::vec3Cross(right, dir, up);
        bx::vec3Norm(right, right);

        pos += dir * speed * delta * (input.forward - input.backward);
        pos += up * speed * delta * (input.up - input.down);
        pos += right * speed * delta * (input.right - input.left);
    }
};

struct CameraID {
    enum {
        FREE_VIEW = 0,
        PLAYER_VIEW,
    } Enum;
};

struct Application {

bool8 mouseCaptured = true;
bool8 showUI = true;
bool8 running = true;
SDL_Window* window;
bgfx::ProgramHandle programTest;
bgfx::ProgramHandle programVertShading;
bgfx::ProgramHandle programVertShadingColor;
bgfx::ProgramHandle programVertShadingColorInstance;
bgfx::ProgramHandle programDbgColor;
bgfx::UniformHandle u_color;
bgfx::VertexBufferHandle cubeVbh;
bgfx::VertexBufferHandle originVbh;
bgfx::VertexBufferHandle gridVbh;
i64 timeOffset;
f64 physWorldTimeAcc = 0;

i32 cameraId = 0;
CameraFreeFlight cam;
mat4 proj;
mat4 camView;

MeshHandle playerShipMesh;
Room room;
PlayerShip playerShip;
PhysBody* playerBody;

f32 dbgRotateX = 0;
f32 dbgRotateY = 0;
f32 dbgRotateZ = 0;
f32 dbgScale = 1;
f32 dbgPlayerCamHeight = 30;
bool dbgEnableGrid = true;
bool dbgEnableWorldOrigin = true;
bool dbgEnableDbgDraw = true;

bool init()
{
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow("Triangle",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT,
                              /*SDL_WINDOW_OPENGL|*/SDL_WINDOW_SHOWN|
                              SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    if(!SDL_GetWindowWMInfo(window, &wmi)) {
        return false;
    }

    bgfx::PlatformData pd;

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    pd.ndt          = wmi.info.x11.display;
    pd.nwh          = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_OSX
    pd.ndt          = NULL;
    pd.nwh          = wmi.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
    pd.ndt          = NULL;
    pd.nwh          = wmi.info.win.window;
#elif BX_PLATFORM_STEAMLINK
    pd.ndt          = wmi.info.vivante.display;
    pd.nwh          = wmi.info.vivante.window;
#endif // BX_PLATFORM_

    pd.context      = NULL;
    pd.backBuffer   = NULL;
    pd.backBufferDS = NULL;
    bgfx::setPlatformData(pd);

    bgfx::Init init;
    init.type = bgfx::RendererType::Direct3D11;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.deviceId = 0;
    init.resolution.width  = 1600;
    init.resolution.height = 900;
    init.resolution.reset = /*BGFX_RESET_NONE*/BGFX_RESET_MSAA_X16;

    if(!bgfx::init(init)) {
        LOG("ERROR> bgfx failed to initialize");
        return false;
    }

    bgfx::setDebug(BGFX_DEBUG_TEXT);

    // Set view 0 clear state.
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

    imguiCreate();

    // Create vertex stream declaration.
    PosColorVertex::init();

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

    u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
    programTest = loadProgram(&g_fileReader, "vs_test", "fs_test");
    programVertShading = loadProgram(&g_fileReader, "vs_vertex_shading", "fs_vertex_shading");
    programVertShadingColor = loadProgram(&g_fileReader, "vs_vertex_shading", "fs_vertex_shading_color");
    programVertShadingColorInstance = loadProgram(&g_fileReader, "vs_vertex_shading_instance",
                                                  "fs_vertex_shading_instance");
    programDbgColor = loadProgram(&g_fileReader, "vs_dbg_color", "fs_dbg_color");

    if(!dbgDrawInit()) {
        return false;
    }

    timeOffset = bx::getHPCounter();

    SDL_SetRelativeMouseMode((SDL_bool)mouseCaptured);

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

    if(!(playerShipMesh = meshLoad("assets/player_ship.mesh", &g_fileReader))) {
        return false;
    }

    initGame();

    return true;
}

void initGame()
{
    room.make(vec3{100, 50, 10}, 4);
    room.tfRoom.pos.z = 3;
    playerShip.tf.pos = {10, 10, 0};

    cameraId = CameraID::PLAYER_VIEW;

    PhysBody body = {};
    Collider colPlayer;
    colPlayer.makeCb(CircleBound{ vec2{0, 0}, 2.0f });
    body.pos = vec2{ 10, 10 };
    body.weight = 1.0;
    body.bounceStrength = 0.0;
    playerBody = room.physWorld.addDynamicBody(colPlayer, body);
    playerShip.body = playerBody;

    Collider col;
    OrientedBoundingBox obb;
    obb.origin = vec2{10, 10};
    obb.size = vec2{10, 20};
    obb.angle = bx::kPiQuarter;
    room.physWorld.addStaticCollider(col.makeObb(obb));

    for(i32 i = 0; i < 100; i++) {
        Collider col;
        CircleBound cb;
        cb.center = vec2{f32(10 + rand01() * 90), f32(10 + rand01() * 40)};
        cb.radius = 1.0 + rand01() * 1;
        PhysBody ball;
        f32 a = rand01() * bx::kPi2;
        f32 speed = 10 + rand01() * 50;
        ball.vel = { cos(a) * speed, sin(a) * speed };
        ball.pos = cb.center;
        ball.bounceStrength = 1.0;
        room.physWorld.addDynamicBody(col.makeCb(cb), ball);
    }
}

void cleanUp()
{
    imguiDestroy();

    dbgDrawDeinit();

    meshUnload(playerShipMesh);
    bgfx::destroy(originVbh);
    bgfx::destroy(gridVbh);
    bgfx::destroy(cubeVbh);
    bgfx::destroy(programTest);
    bgfx::destroy(programVertShading);
    bgfx::destroy(programVertShadingColor);
    bgfx::destroy(programVertShadingColorInstance);
    bgfx::destroy(programDbgColor);
    bgfx::destroy(u_color);

    bgfx::shutdown();
}

i32 run()
{
    i64 t0 = bx::getHPCounter();

    while(running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            handleEvent(event);
        }

        const i64 t1 = bx::getHPCounter();
        const i64 frameTime = t1 - t0;
        t0 = t1;
        const f64 freq = f64(bx::getHPFrequency());
        f64 delta = f64(frameTime/freq);

        update(delta);
        render();
    }

    cleanUp();
    return 0;
}

void handleEvent(const SDL_Event& event)
{
    if(mouseCaptured) {
        if(cameraId == CameraID::FREE_VIEW) {
            cam.handleEvent(event);
        }
        else if(cameraId == CameraID::PLAYER_VIEW) {
            playerShip.handleEvent(event);
        }
    }
    else {
        imguiHandleSDLEvent(event);
    }

    if(event.type == SDL_QUIT) {
        running = false;
        return;
    }

    if(event.type == SDL_KEYDOWN && event.key.repeat == 0) {
        if(event.key.keysym.sym == SDLK_ESCAPE) {
            running = false;
            return;
        }
        if(event.key.keysym.sym == SDLK_w) {
            SDL_SetRelativeMouseMode((SDL_bool)(mouseCaptured ^= 1));
            SDL_WarpMouseInWindow(window, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
            return;
        }
        if(event.key.keysym.sym == SDLK_x) {
            showUI ^= 1;
            return;
        }
        if(event.key.keysym.sym == SDLK_c) {
            cameraId += 1;
            cameraId = cameraId % 2;
            return;
        }
        if(event.key.keysym.sym == SDLK_v) {
            dbgEnableGrid ^= 1;
            dbgEnableWorldOrigin ^= 1;
            return;
        }
    }
}

void updateUI(f64 delta)
{
    i32 mx = 0, my = 0;
    u8 buttons = 0;

    if(!mouseCaptured) {
        u32 mstate = SDL_GetMouseState(&mx, &my);
        if(mstate & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            buttons |= IMGUI_MBUT_LEFT;
        }
        if(mstate & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            buttons |= IMGUI_MBUT_RIGHT;
        }
    }

    if(!showUI) {
        return;
    }

    imguiBeginFrame(mx, my, buttons, WINDOW_WIDTH, WINDOW_HEIGHT);

    // ui code here
    im::ShowDemoWindow();

    im::Begin("Debug");

    const char* comboCameras[] = { "Free flight", "Follow player" };
    im::Combo("Camera", &cameraId, comboCameras, arr_count(comboCameras));
    im::InputFloat("Player camera height", &dbgPlayerCamHeight, 1.0f, 10.0f);
    im::Checkbox("Enable grid", &dbgEnableGrid);
    im::Checkbox("Enable world origin", &dbgEnableWorldOrigin);
    im::Checkbox("Enable debug draw", &dbgEnableDbgDraw);

    im::End();

    im::Begin("Stats");
    const bgfx::Stats* stats = bgfx::getStats();
    const f64 toMsCpu = 1000.0/stats->cpuTimerFreq;
    const f64 toMsGpu = 1000.0/stats->gpuTimerFreq;
    const f64 frameMs = f64(stats->cpuTimeFrame) * toMsCpu;
    const f64 gpuFrameMs = f64(stats->gpuTimeEnd - stats->gpuTimeBegin) * toMsGpu;
    im::Text("CPU ft: %0.5f ms", frameMs);
    im::Text("GPU ft: %0.5f ms", gpuFrameMs);
    im::End();
}

void update(f64 delta)
{
    updateUI(delta);

    physWorldTimeAcc += delta;
    if(physWorldTimeAcc >= PHYS_UPDATE_DELTA) {
#ifdef CONF_DEBUG
        room.physWorld.update(PHYS_UPDATE_DELTA, 2);
#else
        room.physWorld.update(PHYS_UPDATE_DELTA, 10);
#endif
        physWorldTimeAcc = 0;
    }

    /*if(delta > PHYS_UPDATE_DELTA) {
        delta = PHYS_UPDATE_DELTA;
    }
    room.physWorld.update(delta, 10);*/

    dbgDrawLine({0, 0, 0}, {10, 0, 0}, vec4{0, 0, 1, 1}, 0.1f);
    dbgDrawLine({0, 0, 0}, {0, 10, 0}, vec4{0, 1, 0, 1}, 0.1f);
    dbgDrawLine({0, 0, 0}, {0, 0, 10}, vec4{1, 0, 0, 1}, 0.1f);

    playerShip.update(delta, physWorldTimeAcc / PHYS_UPDATE_DELTA);
    playerShip.computeModelMatrix();

    const vec3 up = { 0.0f, 0.0f, 1.0f };
    bx::mtxProjRh(proj, 60.0f, f32(WINDOW_WIDTH)/f32(WINDOW_HEIGHT), 0.1f, 1000.0f,
                  bgfx::getCaps()->homogeneousDepth);

    if(cameraId == CameraID::FREE_VIEW) {
        vec3 at;
        cam.applyInput(delta);

        bx::vec3Add(at, cam.pos, cam.dir);
        bx::mtxLookAtRh(camView, cam.pos, at, up);
    }
    else if(cameraId == CameraID::PLAYER_VIEW) {
        vec3 dir = vec3{0, 0.001f, -1.f};
        bx::vec3Norm(dir, dir);
        vec3 eye = playerShip.tf.pos + vec3{0, 0, dbgPlayerCamHeight};
        vec3 at = eye + dir;
        bx::mtxLookAtRh(camView, eye, at, up);

        mat4 invView;
        mat4 viewProj;
        bx::mtxMul(viewProj, camView, proj);
        bx::mtxInverse(invView, viewProj);
        playerShip.computeCursorPos(invView, dbgPlayerCamHeight);
    }


    f32 time = (f32)((bx::getHPCounter()-timeOffset)/f64(bx::getHPFrequency()));

    room.dbgDraw();

    vec2 v = vec2Rotate({5, 10 }, time);
    dbgDrawLine({0, 0, 0}, vec2ToVec3(v), vec4{1, 1, 1, 1});

    if(showUI) {
        imguiEndFrame();
    }
}

void render()
{
    bgfx::setViewTransform(0, camView, proj);

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, u16(WINDOW_WIDTH), u16(WINDOW_HEIGHT));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

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
        bgfx::submit(0, programDbgColor);
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
        bgfx::submit(0, programDbgColor);
    }

    f32 time = (f32)((bx::getHPCounter()-timeOffset)/f64(bx::getHPFrequency()));

    // debug room
#if 0
    mat4 mtxRoom;
    roomTest.tfRoom.toMtx(&mtxRoom);

    const i32 cubeCount = roomTest.cubeTransforms.size();
    // 80 bytes stride = 64 bytes for 4x4 matrix + 16 bytes for RGBA color.
    const uint16_t instanceStride = 80;
    const uint32_t numInstances   = cubeCount;

    if(numInstances == bgfx::getAvailInstanceDataBuffer(numInstances, instanceStride)) {
        bgfx::InstanceDataBuffer idb;
        bgfx::allocInstanceDataBuffer(&idb, numInstances, instanceStride);

        u8* data = idb.data;

        for(u32 i = 0; i < cubeCount; ++i) {
            Transform tf = roomTest.cubeTransforms[i];
            mat4 mtx1;
            f32* mtxModel = (f32*)data;

            tf.pos.z = tf.pos.z - sin(time + i*0.21f) * 0.2;

            // model * roomModel
            tf.toMtx(&mtx1);
            bx::mtxMul(mtxModel, mtx1, mtxRoom);

            f32* color = (f32*)&data[64];
            color[0] = 0.6f;
            color[1] = 0.4f;
            color[2] = 0.3f;
            color[3] = 1.0f;

            data += instanceStride;
        }

        bgfx::setVertexBuffer(0, cubeVbh, 0, BX_COUNTOF(s_cubeRainbowVertData));
        bgfx::setInstanceDataBuffer(&idb);

        bgfx::setState(0
            | BGFX_STATE_WRITE_MASK
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CCW
            | BGFX_STATE_MSAA
            );

        bgfx::submit(0, programVertShadingColorInstance);
    }
#endif

    // player ship
    const f32 color[] = {1, 0, 0, 1};
    bgfx::setUniform(u_color, color);

    meshSubmit(playerShipMesh, 0, programVertShadingColor, playerShip.mtxModel, BGFX_STATE_MASK);

    // mouse cursor
    if(cameraId == CameraID::PLAYER_VIEW) {
        bgfx::setState(0
            | BGFX_STATE_WRITE_MASK
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CCW
            | BGFX_STATE_MSAA
            );

        const f32 pink[] = {1, 0, 1, 1};
        bgfx::setUniform(u_color, pink);
        mat4 mtxCursor;

        Transform tfCursor;
        tfCursor.pos = playerShip.mousePosWorld;
        tfCursor.scale = { 0.4f, 0.4f, 0.4f };
        tfCursor.toMtx(&mtxCursor);

        bgfx::setTransform(mtxCursor);
        bgfx::setVertexBuffer(0, cubeVbh, 0, BX_COUNTOF(s_cubeRainbowVertData));
        bgfx::submit(0, programDbgColor);
    }

    if(dbgEnableDbgDraw) {
        dbgDrawRender();
    }

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}

};

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#else
int main()
#endif
{
    SDL_SetMainReady();

    i32 sdl = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS);
    if(sdl != 0) {
        LOG("ERROR> could not init SDL2 (%s)", SDL_GetError());
        return 1;
    }

    Application app;
    if(!app.init()) {
        LOG("ERROR> could not init application.");
        return 1;
    }
    i32 r = app.run();

    SDL_Quit();
    return r;
}
