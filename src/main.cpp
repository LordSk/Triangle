#ifdef _WIN32
    #define SDL_MAIN_HANDLED
    #include <windows.h>
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "base.h"
#include <time.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <bx/file.h>
#include <bx/timer.h>
#include <bx/rng.h>
#include <imgui/bgfx_imgui.h>

#define STATIC_MESH_DATA
#include "static_mesh.h"

#include "mesh_load.h"
#include "utils.h"
#include "dbg_draw.h"
#include "collision.h"
#include "game.h"
#include "renderer.h"
#include "input_recorder.h"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

namespace im = ImGui;

// TODO: move this
static bx::FileReader g_fileReader;
static bx::FileWriter g_fileWriter;

struct Application {

bool8 mouseCaptured = true;
bool8 showUI = true;
bool8 running = true;
SDL_Window* window;

i64 timeOffset;
f64 physWorldTimeAcc = 0;

Renderer renderer; // TODO: make global singleton
GameData game;

f32 dbgRotateX = 0;
f32 dbgRotateY = 0;
f32 dbgRotateZ = 0;
f32 dbgScale = 1;

bool init()
{
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow("Project T",
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

    timeOffset = bx::getHPCounter();

    SDL_SetRelativeMouseMode((SDL_bool)mouseCaptured);

    if(!renderer.init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
        LOG("ERROR> could not init renderer");
        return false;
    }
    setRendererGlobal(&renderer);

    if(!dbgDrawInit()) {
        return false;
    }

    game.init();

    imguiCreate();

    return true;
}

void cleanUp()
{
    game.deinit();

    imguiDestroy();

    dbgDrawDeinit();

    renderer.deinit();
}

i32 run()
{
    i64 t0 = bx::getHPCounter();

    while(running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            handleEvent(event);
        }

        const f64 freq = f64(bx::getHPFrequency());
        i64 t1 = bx::getHPCounter();
        i64 frameTime = t1 - t0;
        f64 delta = f64(frameTime/freq);

        // limit fps
        while(delta < 1.0/120.0) {
            t1 = bx::getHPCounter();
            frameTime = t1 - t0;
            delta = f64(frameTime/freq);
            _mm_pause();
        }

        t0 = t1;

        update(delta);

        if(showUI) {
            imguiEndFrame();
        }
        else {
            ImGui::EndFrame(); // don't render
        }

        render();

        renderer.frame();
    }

    cleanUp();
    return 0;
}

void handleEvent(const SDL_Event& event)
{
    game.handlEvent(event);

    if(mouseCaptured) {
        inputRecorderHandleEvent(event);
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
        if(event.key.keysym.sym == SDLK_v) {
            renderer.dbgEnableGrid ^= 1;
            renderer.dbgEnableWorldOrigin ^= 1;
            return;
        }
        if(event.key.keysym.sym == SDLK_p) {
            renderer.dbgEnableDbgPhysics ^= 1;
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

    imguiBeginFrame(mx, my, buttons, WINDOW_WIDTH, WINDOW_HEIGHT, 0xff, 1 /* RdrViewID::UI */);

    // ui code here
    //im::ShowDemoWindow();

    im::Begin("Debug");

    const char* comboCameras[] = { "Free flight", "Follow player" };
    im::Combo("Camera", &game.cameraId, comboCameras, arr_count(comboCameras));
    im::InputFloat("Player camera height", &game.dbgPlayerCamHeight, 1.0f, 10.0f);
    im::Checkbox("Enable grid", &renderer.dbgEnableGrid);
    im::Checkbox("Enable world origin", &renderer.dbgEnableWorldOrigin);
    im::Checkbox("Enable debug physics", &renderer.dbgEnableDbgPhysics);
    im::Checkbox("Enable debug damage zones", &game.dbgEnableDmgZones);

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

    game.update(delta);
    game.mouseCaptured = mouseCaptured;

    if(renderer.dbgEnableWorldOrigin) {
        dbgDrawLine({0, 0, 0}, {10, 0, 0}, vec4{0, 0, 1, 1}, 0.1f);
        dbgDrawLine({0, 0, 0}, {0, 10, 0}, vec4{0, 1, 0, 1}, 0.1f);
        dbgDrawLine({0, 0, 0}, {0, 0, 10}, vec4{1, 0, 0, 1}, 0.1f);
    }
}

void render()
{
    game.render();

    dbgDrawRender();
}

};

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#else
int main()
#endif
{
    randSetSeed(time(0));

#if 0
    ArraySparse<u32> testSparse;
    for(i32 i = 0; i < 100; i++) {
        testSparse.push((u32)i);
    }

    for(i32 i = 0; i < testSparse.count(); i++) {
        LOG("[%d] = %d", i, testSparse[i]);
    }

    LOG("count before remove = %d", testSparse.count());

    for(i32 i = 0; i < 100; i++) {
        testSparse.removeById(xorshift32() % 100);
    }

    LOG("count after remove = %d", testSparse.count());

    for(i32 i = 0; i < 100; i++) {
        testSparse.emplace(i, (u32)i);
    }

    for(i32 i = 0; i < testSparse.count(); i++) {
        LOG("[%d] = %d", i, testSparse[i]);
    }

    return 0;
#endif

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
