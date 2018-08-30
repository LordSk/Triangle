#include "bgfx_imgui.h"

/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <bx/allocator.h>
#include <bx/math.h>
#include <bx/timer.h>
#include "imgui.h"

#include "vs_ocornut_imgui.bin.h"
#include "fs_ocornut_imgui.bin.h"
#include "vs_imgui_image.bin.h"
#include "fs_imgui_image.bin.h"

#include "roboto_regular.ttf.h"
#include "robotomono_regular.ttf.h"

#include <SDL2/SDL_scancode.h>

static const bgfx::EmbeddedShader s_embeddedShaders[] =
{
    BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
    BGFX_EMBEDDED_SHADER(fs_ocornut_imgui),
    BGFX_EMBEDDED_SHADER(vs_imgui_image),
    BGFX_EMBEDDED_SHADER(fs_imgui_image),

    BGFX_EMBEDDED_SHADER_END()
};

static void* memAlloc(size_t _size, void* _userData);
static void memFree(void* _ptr, void* _userData);

inline bool checkAvailTransientBuffers(uint32_t _numVertices, const bgfx::VertexDecl& _decl,
                                       uint32_t _numIndices)
{
    return _numVertices == bgfx::getAvailTransientVertexBuffer(_numVertices, _decl)
        && (0 == _numIndices || _numIndices == bgfx::getAvailTransientIndexBuffer(_numIndices) )
        ;
}

struct OcornutImguiContext
{
    void render(ImDrawData* _drawData)
    {
        const ImGuiIO& io = ImGui::GetIO();
        const float width  = io.DisplaySize.x;
        const float height = io.DisplaySize.y;

        bgfx::setViewName(m_viewId, "ImGui");
        bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);

        const bgfx::HMD*  hmd  = bgfx::getHMD();
        const bgfx::Caps* caps = bgfx::getCaps();
        if (NULL != hmd && 0 != (hmd->flags & BGFX_HMD_RENDERING) )
        {
            float proj[16];
            bx::mtxProj(proj, hmd->eye[0].fov, 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);

            static float time = 0.0f;
            time += 0.05f;

            const float dist = 10.0f;
            const float offset0 = -proj[8] + (hmd->eye[0].viewOffset[0] / dist * proj[0]);
            const float offset1 = -proj[8] + (hmd->eye[1].viewOffset[0] / dist * proj[0]);

            float ortho[2][16];
            const float viewOffset = width/4.0f;
            const float viewWidth  = width/2.0f;
            bx::mtxOrtho(ortho[0], viewOffset, viewOffset + viewWidth, height, 0.0f, 0.0f, 1000.0f, offset0, caps->homogeneousDepth);
            bx::mtxOrtho(ortho[1], viewOffset, viewOffset + viewWidth, height, 0.0f, 0.0f, 1000.0f, offset1, caps->homogeneousDepth);
            bgfx::setViewTransform(m_viewId, NULL, ortho[0], BGFX_VIEW_STEREO, ortho[1]);
            bgfx::setViewRect(m_viewId, 0, 0, hmd->width, hmd->height);
        }
        else
        {
            float ortho[16];
            bx::mtxOrtho(ortho, 0.0f, width, height, 0.0f, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
            bgfx::setViewTransform(m_viewId, NULL, ortho);
            bgfx::setViewRect(m_viewId, 0, 0, uint16_t(width), uint16_t(height) );
        }

        // Render command lists
        for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii)
        {
            bgfx::TransientVertexBuffer tvb;
            bgfx::TransientIndexBuffer tib;

            const ImDrawList* drawList = _drawData->CmdLists[ii];
            uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
            uint32_t numIndices  = (uint32_t)drawList->IdxBuffer.size();

            if (!checkAvailTransientBuffers(numVertices, m_decl, numIndices) )
            {
                // not enough space in transient buffer just quit drawing the rest...
                break;
            }

            bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_decl);
            bgfx::allocTransientIndexBuffer(&tib, numIndices);

            ImDrawVert* verts = (ImDrawVert*)tvb.data;
            bx::memCopy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert) );

            ImDrawIdx* indices = (ImDrawIdx*)tib.data;
            bx::memCopy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx) );

            uint32_t offset = 0;
            for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
            {
                if (cmd->UserCallback)
                {
                    cmd->UserCallback(drawList, cmd);
                }
                else if (0 != cmd->ElemCount)
                {
                    uint64_t state = 0
                        | BGFX_STATE_WRITE_RGB
                        | BGFX_STATE_WRITE_A
                        | BGFX_STATE_MSAA
                        ;

                    bgfx::TextureHandle th = m_texture;
                    bgfx::ProgramHandle program = m_program;

                    if (NULL != cmd->TextureId)
                    {
                        union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
                        state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
                            ? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                            : BGFX_STATE_NONE
                            ;
                        th = texture.s.handle;
                        if (0 != texture.s.mip)
                        {
                            const float lodEnabled[4] = { float(texture.s.mip), 1.0f, 0.0f, 0.0f };
                            bgfx::setUniform(u_imageLodEnabled, lodEnabled);
                            program = m_imageProgram;
                        }
                    }
                    else
                    {
                        state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
                    }

                    const uint16_t xx = uint16_t(bx::max(cmd->ClipRect.x, 0.0f) );
                    const uint16_t yy = uint16_t(bx::max(cmd->ClipRect.y, 0.0f) );
                    bgfx::setScissor(xx, yy
                            , uint16_t(bx::min(cmd->ClipRect.z, 65535.0f)-xx)
                            , uint16_t(bx::min(cmd->ClipRect.w, 65535.0f)-yy)
                            );

                    bgfx::setState(state);
                    bgfx::setTexture(0, s_tex, th);
                    bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
                    bgfx::setIndexBuffer(&tib, offset, cmd->ElemCount);
                    bgfx::submit(m_viewId, program);
                }

                offset += cmd->ElemCount;
            }
        }
    }

    void create(float _fontSize, bx::AllocatorI* _allocator)
    {
        m_allocator = _allocator;

        if (NULL == _allocator)
        {
            static bx::DefaultAllocator allocator;
            m_allocator = &allocator;
        }

        m_viewId = 255;
        m_last = bx::getHPCounter();

        ImGui::SetAllocatorFunctions(memAlloc, memFree, NULL);

        m_imgui = ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2(1280.0f, 720.0f);
        io.DeltaTime   = 1.0f / 60.0f;
        //io.IniFilename = "imgui.ini";

        setupStyle(true);

        io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
        io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
        io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
        io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
        io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
        io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
        io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
        io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
        io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
        io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
        io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
        io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
        io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
        io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

#if 0
        io.ConfigFlags |= 0
            | ImGuiConfigFlags_NavEnableGamepad
            | ImGuiConfigFlags_NavEnableKeyboard
            ;

        io.NavInputs[ImGuiNavInput_Activate]    = (int)entry::Key::GamepadA;
        io.NavInputs[ImGuiNavInput_Cancel]      = (int)entry::Key::GamepadB;
//		io.NavInputs[ImGuiNavInput_Input]       = (int)entry::Key::;
//		io.NavInputs[ImGuiNavInput_Menu]        = (int)entry::Key::;
        io.NavInputs[ImGuiNavInput_DpadLeft]    = (int)entry::Key::GamepadLeft;
        io.NavInputs[ImGuiNavInput_DpadRight]   = (int)entry::Key::GamepadRight;
        io.NavInputs[ImGuiNavInput_DpadUp]      = (int)entry::Key::GamepadUp;
        io.NavInputs[ImGuiNavInput_DpadDown]    = (int)entry::Key::GamepadDown;
//		io.NavInputs[ImGuiNavInput_LStickLeft]  = (int)entry::Key::;
//		io.NavInputs[ImGuiNavInput_LStickRight] = (int)entry::Key::;
//		io.NavInputs[ImGuiNavInput_LStickUp]    = (int)entry::Key::;
//		io.NavInputs[ImGuiNavInput_LStickDown]  = (int)entry::Key::;
//		io.NavInputs[ImGuiNavInput_FocusPrev]   = (int)entry::Key::;
//		io.NavInputs[ImGuiNavInput_FocusNext]   = (int)entry::Key::;
//		io.NavInputs[ImGuiNavInput_TweakSlow]   = (int)entry::Key::;
//		io.NavInputs[ImGuiNavInput_TweakFast]   = (int)entry::Key::;
#endif

        bgfx::RendererType::Enum type = bgfx::getRendererType();
        m_program = bgfx::createProgram(
              bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_ocornut_imgui")
            , bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_ocornut_imgui")
            , true
            );

        u_imageLodEnabled = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
        m_imageProgram = bgfx::createProgram(
              bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_imgui_image")
            , bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_imgui_image")
            , true
            );

        m_decl
            .begin()
            .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
            .end();

        s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Int1);

        uint8_t* data;
        int32_t width;
        int32_t height;
        {
            ImFontConfig config;
            config.FontDataOwnedByAtlas = false;
            config.MergeMode = false;
//			config.MergeGlyphCenterV = true;

            const ImWchar* ranges = io.Fonts->GetGlyphRangesCyrillic();
            m_font[ImGui::Font::Regular] = io.Fonts->AddFontFromMemoryTTF( (void*)s_robotoRegularTtf,     sizeof(s_robotoRegularTtf),     _fontSize,      &config, ranges);
            m_font[ImGui::Font::Mono   ] = io.Fonts->AddFontFromMemoryTTF( (void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf), _fontSize-3.0f, &config, ranges);

            config.MergeMode = true;
            config.DstFont   = m_font[ImGui::Font::Regular];
        }

        io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

        m_texture = bgfx::createTexture2D(
              (uint16_t)width
            , (uint16_t)height
            , false
            , 1
            , bgfx::TextureFormat::BGRA8
            , 0
            , bgfx::copy(data, width*height*4)
            );

        //ImGui::InitDockContext();
    }

    void destroy()
    {
        //ImGui::ShutdownDockContext();
        ImGui::DestroyContext(m_imgui);

        bgfx::destroy(s_tex);
        bgfx::destroy(m_texture);

        bgfx::destroy(u_imageLodEnabled);
        bgfx::destroy(m_imageProgram);
        bgfx::destroy(m_program);

        m_allocator = NULL;
    }

    void setupStyle(bool _dark)
    {
        // Doug Binks' darl color scheme
        // https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9
        ImGuiStyle& style = ImGui::GetStyle();
        if (_dark)
        {
            ImGui::StyleColorsDark(&style);
        }
        else
        {
            ImGui::StyleColorsLight(&style);
        }

        style.FrameRounding    = 4.0f;
        style.WindowBorderSize = 0.0f;
    }

    void beginFrame(
          int32_t _mx
        , int32_t _my
        , uint8_t _button
        , int _width
        , int _height
        , char _inputChar
        , bgfx::ViewId _viewId
        )
    {
        m_viewId = _viewId;

        ImGuiIO& io = ImGui::GetIO();
        /*if (_inputChar < 0x7f)
        {
            io.AddInputCharacter(_inputChar); // ASCII or GTFO! :(
        }*/

        io.DisplaySize = ImVec2( (float)_width, (float)_height);

        const int64_t now = bx::getHPCounter();
        const int64_t frameTime = now - m_last;
        m_last = now;
        const double freq = double(bx::getHPFrequency() );
        io.DeltaTime = float(frameTime/freq);

        io.MousePos = ImVec2( (float)_mx, (float)_my);
        io.MouseDown[0] = 0 != (_button & IMGUI_MBUT_LEFT);
        io.MouseDown[1] = 0 != (_button & IMGUI_MBUT_RIGHT);
        io.MouseDown[2] = 0 != (_button & IMGUI_MBUT_MIDDLE);

#if USE_ENTRY
        uint8_t modifiers = inputGetModifiersState();
        io.KeyShift = 0 != (modifiers & (entry::Modifier::LeftShift | entry::Modifier::RightShift) );
        io.KeyCtrl  = 0 != (modifiers & (entry::Modifier::LeftCtrl  | entry::Modifier::RightCtrl ) );
        io.KeyAlt   = 0 != (modifiers & (entry::Modifier::LeftAlt   | entry::Modifier::RightAlt  ) );
        for (int32_t ii = 0; ii < (int32_t)entry::Key::Count; ++ii)
        {
            io.KeysDown[ii] = inputGetKeyState(entry::Key::Enum(ii) );
        }
#endif // USE_ENTRY

        ImGui::NewFrame();

        // TODO: add guizmo
        // https://github.com/CedricGuillemet/ImGuizmo
        //ImGuizmo::BeginFrame();
    }

    void endFrame()
    {
        ImGui::Render();
        render(ImGui::GetDrawData() );
    }

    ImGuiContext*       m_imgui;
    bx::AllocatorI*     m_allocator;
    bgfx::VertexDecl    m_decl;
    bgfx::ProgramHandle m_program;
    bgfx::ProgramHandle m_imageProgram;
    bgfx::TextureHandle m_texture;
    bgfx::UniformHandle s_tex;
    bgfx::UniformHandle u_imageLodEnabled;
    ImFont* m_font[ImGui::Font::Count];
    int64_t m_last;
    bgfx::ViewId m_viewId;
};

static OcornutImguiContext s_ctx;

static void* memAlloc(size_t _size, void* _userData)
{
    BX_UNUSED(_userData);
    return BX_ALLOC(s_ctx.m_allocator, _size);
}

static void memFree(void* _ptr, void* _userData)
{
    BX_UNUSED(_userData);
    BX_FREE(s_ctx.m_allocator, _ptr);
}

void imguiCreate(float _fontSize, bx::AllocatorI* _allocator)
{
    s_ctx.create(_fontSize, _allocator);
}

void imguiDestroy()
{
    s_ctx.destroy();
}

void imguiBeginFrame(int32_t _mx, int32_t _my, uint8_t _button, uint16_t _width,
                     uint16_t _height, char _inputChar, bgfx::ViewId _viewId)
{
    s_ctx.beginFrame(_mx, _my, _button, _width, _height, _inputChar, _viewId);
}

void imguiEndFrame()
{
    s_ctx.endFrame();
}

void imguiHandleSDLEvent(const SDL_Event& event)
{
    ImGuiIO& io = ImGui::GetIO();

    if(event.type == SDL_MOUSEWHEEL) {
        io.MouseWheel = event.wheel.y;
        return;
    }

    if(event.type == SDL_KEYDOWN) {
        io.KeysDown[event.key.keysym.scancode] = true;

        if(event.key.keysym.scancode == SDL_SCANCODE_KP_ENTER) {
            io.KeysDown[io.KeyMap[ImGuiKey_Enter]] = true;
        }

        if(event.key.keysym.scancode == SDL_SCANCODE_LCTRL ||
           event.key.keysym.scancode == SDL_SCANCODE_RCTRL) {
            io.KeyCtrl = true;
        }
        if(event.key.keysym.scancode == SDL_SCANCODE_LSHIFT ||
           event.key.keysym.scancode == SDL_SCANCODE_RSHIFT) {
            io.KeyShift = true;
        }
        if(event.key.keysym.scancode == SDL_SCANCODE_LALT ||
           event.key.keysym.scancode == SDL_SCANCODE_RALT) {
            io.KeyAlt = true;
        }

        if(event.key.keysym.mod & KMOD_CTRL) {
            io.KeyCtrl = true;
        }
        if(event.key.keysym.mod & KMOD_ALT) {
            io.KeyAlt = true;
        }
        if(event.key.keysym.mod & KMOD_SHIFT) {
            io.KeyShift = true;
        }
        return;
    }

    if(event.type == SDL_KEYUP) {
        io.KeysDown[event.key.keysym.scancode] = false;

        if(event.key.keysym.scancode == SDL_SCANCODE_KP_ENTER) {
            io.KeysDown[io.KeyMap[ImGuiKey_Enter]] = false;
        }

        if(event.key.keysym.scancode == SDL_SCANCODE_LCTRL ||
           event.key.keysym.scancode == SDL_SCANCODE_RCTRL) {
            io.KeyCtrl = false;
        }
        if(event.key.keysym.scancode == SDL_SCANCODE_LSHIFT ||
           event.key.keysym.scancode == SDL_SCANCODE_RSHIFT) {
            io.KeyShift = false;
        }
        if(event.key.keysym.scancode == SDL_SCANCODE_LALT ||
           event.key.keysym.scancode == SDL_SCANCODE_RALT) {
            io.KeyAlt = false;
        }
        return;
    }

    if(event.type == SDL_TEXTINPUT) {
        io.AddInputCharactersUTF8(event.text.text);
        return;
    }
}

namespace ImGui
{
    void PushFont(Font::Enum _font)
    {
        PushFont(s_ctx.m_font[_font]);
    }
} // namespace ImGui

BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4505); // error C4505: '' : unreferenced local function has been removed
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-function"); // warning: ‘int rect_width_compare(const void*, const void*)’ defined but not used
BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunknown-pragmas")
//BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-but-set-variable"); // warning: variable ‘L1’ set but not used
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wtype-limits"); // warning: comparison is always true due to limited range of data type
#define STBTT_malloc(_size, _userData) memAlloc(_size, _userData)
#define STBTT_free(_ptr, _userData) memFree(_ptr, _userData)
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
BX_PRAGMA_DIAGNOSTIC_POP();
