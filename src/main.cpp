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

#include "static_mesh.h"
#include "mesh_load.h"
#include "player_ship.h"

#include <vector>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

// TODO: move this

bx::FileReader g_fileReader;
bx::FileWriter g_fileWriter;
bx::RngMwc g_rng;

static const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath)
{
    if(bx::open(_reader, _filePath)) {
        uint32_t size = (uint32_t)bx::getSize(_reader);
        const bgfx::Memory* mem = bgfx::alloc(size+1);
        bx::read(_reader, mem->data, size);
        bx::close(_reader);
        mem->data[mem->size-1] = '\0';
        return mem;
    }

    LOG("Failed to load %s.", _filePath);
    assert(0);
    return NULL;
}

static bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const char* _name)
{
    char filePath[512];

    const char* ext = "???";

    switch(bgfx::getRendererType()) {
        case bgfx::RendererType::Noop:
        case bgfx::RendererType::Direct3D9:  ext = ".dx9";   break;
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12: ext = ".dx11";  break;
        case bgfx::RendererType::Gnm:        ext = ".pssl";  break;
        case bgfx::RendererType::Metal:      ext = ".metal"; break;
        case bgfx::RendererType::OpenGL:     ext = ".glsl";  break;
        case bgfx::RendererType::OpenGLES:   ext = ".essl";  break;
        case bgfx::RendererType::Vulkan:     ext = ".spirv"; break;

        case bgfx::RendererType::Count:
            assert(0);
            break;
    }

    bx::strCopy(filePath, BX_COUNTOF(filePath), _name);
    bx::strCat(filePath, BX_COUNTOF(filePath), ext);

    bgfx::ShaderHandle handle = bgfx::createShader(loadMem(_reader, filePath) );
    bgfx::setName(handle, filePath);

    return handle;
}

bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName)
{
    bgfx::ShaderHandle vsh = loadShader(_reader, _vsName);
    bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
    if (NULL != _fsName)
    {
        fsh = loadShader(_reader, _fsName);
    }

    return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
}

bgfx::VertexDecl PosColorVertex::ms_decl;

struct Transform
{
    vec3 pos = {0, 0, 0};
    vec3 scale = {1, 1, 1};
    quat rot = {0, 0, 0, 1};

    inline void toMtx(mat4* mtxModel) {
        mat4 mtxTrans, mtxRot, mtxScale;

        bx::mtxTranslate(mtxTrans, pos.x, pos.y, pos.z);
        bx::mtxQuat(mtxRot, rot);
        bx::mtxScale(mtxScale, scale.x, scale.y, scale.z);

        bx::mtxMul(*mtxModel, mtxScale, mtxRot);
        bx::mtxMul(*mtxModel, *mtxModel, mtxTrans);
    }
};

struct Room
{
    vec3 size;
    std::vector<Transform> cubeTransforms;

    void make(vec3 size_) {
        size = vec3{bx::ceil(size_.x), bx::ceil(size_.y), bx::ceil(size_.z)};
        i32 width = size.x/2;
        i32 height = size.y/2;

        for(i32 y = 0; y < width; y++) {
            for(i32 x = 0; x < height; x++) {
                Transform tf;
                tf.pos.x = x * 2;
                tf.pos.y = y * 2;
                tf.pos.z = bx::frnd(&g_rng);
                tf.scale = {1, 1, 1};
                bx::quatIdentity(tf.rot);
                cubeTransforms.push_back(tf);
            }
        }
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

struct Application {

bool8 mouseCaptured = true;
bool8 running = true;
SDL_Window* window;
bgfx::ProgramHandle programTest;
bgfx::ProgramHandle programVertShading;
bgfx::ProgramHandle programVertShadingColor;
bgfx::ProgramHandle programDbgColor;
bgfx::UniformHandle u_color;
bgfx::VertexBufferHandle cubeVbh;
bgfx::VertexBufferHandle originVbh;
bgfx::VertexBufferHandle gridVbh;
i64 timeOffset;

MeshHandle playerShipMesh;

f32 dbgRotateX = 0;
f32 dbgRotateY = 0;
f32 dbgRotateZ = 0;
f32 dbgScale = 1;
i32 dbgCamComboId = 0;
f32 dbgPlayerCamHeight = 15;

Room roomTest;
CameraFreeFlight cam;
PlayerShip playerShip;

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
    init.resolution.reset = BGFX_RESET_NONE;

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
    programDbgColor = loadProgram(&g_fileReader, "vs_dbg_color", "fs_dbg_color");

    timeOffset = bx::getHPCounter();

    if(!(playerShipMesh = meshLoad("assets/player_ship.mesh", &g_fileReader))) {
        return false;
    }

    roomTest.make(vec3{30, 20, 5});

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

    return true;
}

void cleanUp()
{
    imguiDestroy();

    meshUnload(playerShipMesh);
    bgfx::destroy(cubeVbh);
    bgfx::destroy(programTest);
    bgfx::destroy(programVertShading);
    bgfx::destroy(programVertShadingColor);
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
    }

    cleanUp();
    return 0;
}

void handleEvent(const SDL_Event& event)
{
    if(mouseCaptured) {
        cam.handleEvent(event);
        playerShip.handleEvent(event);
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
        if(event.key.keysym.sym == SDLK_c) {
            dbgCamComboId += 1;
            dbgCamComboId = dbgCamComboId % 2;
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

    imguiBeginFrame(mx, my, buttons, WINDOW_WIDTH, WINDOW_HEIGHT);

    // ui code here
    ImGui::ShowDemoWindow();

    ImGui::Begin("Debug");

    const char* comboCameras[] = { "Free flight", "Follow player" };
    ImGui::Combo("Camera", &dbgCamComboId, comboCameras, arr_count(comboCameras));
    ImGui::InputFloat("Player camera height", &dbgPlayerCamHeight, 1.0f, 10.0f);
    ImGui::End();
}

void update(f64 delta)
{
    updateUI(delta);

    playerShip.update(delta);

    const vec3 up = { 0.0f, 0.0f, 1.0f };
    mat4 view;

    if(dbgCamComboId == 0) {
        vec3 at;
        cam.applyInput(delta);
        bx::vec3Add(at, cam.pos, cam.dir);
        bx::mtxLookAtRh(view, cam.pos, at, up);
    }
    else if(dbgCamComboId == 1) {
        vec3 dir = vec3{0, 0.001f, -1.f};
        bx::vec3Norm(dir, dir);
        vec3 eye = playerShip.pos + vec3{0, 0, dbgPlayerCamHeight};
        vec3 at = eye + dir;
        bx::mtxLookAtRh(view, eye, at, up);
    }

    mat4 proj;
    bx::mtxProjRh(proj, 60.0f, f32(WINDOW_WIDTH)/f32(WINDOW_HEIGHT), 0.1f, 1000.0f,
                  bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view, proj);

    mat4 invView;
    mat4 viewProj;
    bx::mtxMul(viewProj, view, proj);
    bx::mtxInverse(invView, viewProj);
    playerShip.computeCursorPos(invView, dbgPlayerCamHeight);

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, u16(WINDOW_WIDTH), u16(WINDOW_HEIGHT));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

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

    float time = (float)( (bx::getHPCounter()-timeOffset)/double(bx::getHPFrequency() ) );

    /*for(u32 yy = 0; yy < 11; ++yy) {
        for(u32 xx = 0; xx < 11; ++xx) {
            f32 mtx[16];
            bx::mtxIdentity(mtx);
            bx::mtxRotateXY(mtx, time + xx*0.21f, time + yy*0.37f);
            //bx::mtxRotateXYZ(mtx, dbgRotateX, dbgRotateY, dbgRotateZ);
            mtx[12] = -15.0f + f32(xx)*3.0f;
            mtx[13] = 0.0f;
            mtx[14] = -15.0f + f32(yy)*3.0f;

            // Set model matrix for rendering.
            bgfx::setTransform(mtx);

            // Set vertex and index buffer.
            bgfx::setVertexBuffer(0, cubeVbh, 0, BX_COUNTOF(s_cubeRainbowVertData));

            // Set render states.
            bgfx::setState(0
                | BGFX_STATE_WRITE_MASK
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
                //| BGFX_STATE_PT_TRIANGLES (is triangles by default)
                );

            // Submit primitive for rendering to view 0.
            const f32 color[] = {1, 1, 1, 1};
            bgfx::setUniform(u_color, color);
            bgfx::submit(0, programVertShadingColor);
        }
    }*/


    const i32 cubeCount = roomTest.cubeTransforms.size();
    for(u32 i = 0; i < cubeCount; ++i) {
        const Transform& tf = roomTest.cubeTransforms[i];
        mat4 mtxModel;
        mat4 mtxTrans, mtxRot, mtxScale;

        f32 z = -4 + tf.pos.z + sin(time + i*0.21f) * 0.2;

        bx::mtxTranslate(mtxTrans, tf.pos.x, tf.pos.y, z);
        bx::mtxQuat(mtxRot, tf.rot);
        bx::mtxScale(mtxScale, tf.scale.x, tf.scale.y, tf.scale.z);

        bx::mtxMul(mtxModel, mtxScale, mtxRot);
        bx::mtxMul(mtxModel, mtxModel, mtxTrans);

        // Set model matrix for rendering.
        bgfx::setTransform(mtxModel);

        // Set vertex and index buffer.
        bgfx::setVertexBuffer(0, cubeVbh, 0, BX_COUNTOF(s_cubeRainbowVertData));

        // Set render states.
        bgfx::setState(0
            | BGFX_STATE_WRITE_MASK
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CCW
            | BGFX_STATE_MSAA
            );

        // Submit primitive for rendering to view 0.
        const f32 color[] = {1, 0.5, 0, 1};
        bgfx::setUniform(u_color, color);
        bgfx::submit(0, programVertShadingColor);
    }

    const f32 color[] = {1, 0, 0, 1};
    bgfx::setUniform(u_color, color);

    playerShip.computeModelMatrix();
    meshSubmit(playerShipMesh, 0, programVertShadingColor, playerShip.mtxModel, BGFX_STATE_MASK);

    if(dbgCamComboId == 1) {
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
        tfCursor.scale = { 0.2f, 0.2f, 0.2f };
        tfCursor.toMtx(&mtxCursor);

        bgfx::setTransform(mtxCursor);
        bgfx::setVertexBuffer(0, cubeVbh, 0, BX_COUNTOF(s_cubeRainbowVertData));
        bgfx::submit(0, programDbgColor);
    }

    imguiEndFrame();

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
