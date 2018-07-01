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
#include <imgui/bgfx_imgui.h>

#include "static_mesh.h"
#include "mesh_load.h"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

// TODO: move this

bx::FileReader g_fileReader;
bx::FileWriter g_fileWriter;

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

    switch (bgfx::getRendererType() )
    {
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

struct Application {

bool running = true;
SDL_Window* window;
bgfx::ProgramHandle programTest;
bgfx::ProgramHandle programVertShading;
bgfx::ProgramHandle programVertShadingColor;
bgfx::ProgramHandle programDbgColor;
bgfx::UniformHandle u_color;
bgfx::VertexBufferHandle cubeVbh;
bgfx::VertexBufferHandle originVbh;
i64 timeOffset;

MeshHandle playerShipMesh;

f32 dbgRotateX;
f32 dbgRotateY;
f32 dbgRotateZ;
f32 dbgScale = 1;

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
    while(running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            handleEvent(event);
        }

        update();
    }

    cleanUp();
    return 0;
}

void handleEvent(const SDL_Event& event)
{
    imguiHandleSDLEvent(event);

    if(event.type == SDL_QUIT) {
        running = false;
        return;
    }

    if(event.type == SDL_KEYDOWN) {
        if(event.key.keysym.sym == SDLK_ESCAPE) {
            running = false;
            return;
        }
    }
}

void update()
{
    i32 mx, my;
    u32 mstate = SDL_GetMouseState(&mx, &my);
    u8 buttons = 0;
    if(mstate & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        buttons |= IMGUI_MBUT_LEFT;
    }
    if(mstate & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        buttons |= IMGUI_MBUT_RIGHT;
    }

    // TODO: add scroll
    imguiBeginFrame(mx, my, buttons, WINDOW_WIDTH, WINDOW_HEIGHT);

    // ui code here
    ImGui::ShowDemoWindow();

    ImGui::Begin("Test");
    ImGui::SliderAngle("x", &dbgRotateX);
    ImGui::SliderAngle("y", &dbgRotateY);
    ImGui::SliderAngle("z", &dbgRotateZ);
    ImGui::SliderFloat("scale", &dbgScale, 0.1, 100);
    ImGui::End();

    imguiEndFrame();


    float at[3]  = { 0.0f, 0.0f,   0.0f };
    float eye[3] = { 10.0f, -35.0f, 10.0f };
    float up[3] =  { 0.0f, 0.0f, 1.0f };
    float view[16];
    bx::mtxLookAtRh(view, eye, at, up);

    float proj[16];
    bx::mtxProjRh(proj, 60.0f, f32(WINDOW_WIDTH)/f32(WINDOW_HEIGHT), 0.1f, 100.0f,
                bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view, proj);

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

    float time = (float)( (bx::getHPCounter()-timeOffset)/double(bx::getHPFrequency() ) );

    for(u32 yy = 0; yy < 11; ++yy) {
        for(u32 xx = 0; xx < 11; ++xx) {
            f32 mtx[16];
            bx::mtxIdentity(mtx);
            bx::mtxRotateXY(mtx, time + xx*0.21f, time + yy*0.37f);
            //bx::mtxRotateZ(mtx, dbgRotate);
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
    }

    const f32 color[] = {1, 0, 0, 1};
    bgfx::setUniform(u_color, color);

    f32 mtx[16];
    bx::mtxRotateXYZ(mtx, dbgRotateX, dbgRotateY, dbgRotateZ);
    f32 mtxScale[16];
    bx::mtxScale(mtxScale, dbgScale);
    bx::mtxMul(mtx, mtx, mtxScale);
    meshSubmit(playerShipMesh, 0, programVertShadingColor, mtx, BGFX_STATE_MASK);

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
