#include "input_recorder.h"

struct VkeyLink
{
    enum Type: i32 {
        Keyboard=0,
        Mouse
    };

    Type type;
    union { i32 key; i32 button; };
};

constexpr VkeyLink g_vkeySdlKey[Vkey::_Count] {
    {},
    { VkeyLink::Keyboard, SDLK_q },
    { VkeyLink::Keyboard, SDLK_d },
    { VkeyLink::Keyboard, SDLK_z },
    { VkeyLink::Keyboard, SDLK_s },
    { VkeyLink::Mouse, SDL_BUTTON_LEFT },
};

struct InputRecorder
{
    bool8 vkeyStatus[Vkey::_Count] = {0};
    f32 mouseX = 0;
    f32 mouseY = 0;

    void handleEvent(const SDL_Event& event)
    {
        if(event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            const i32 sdlk = event.key.keysym.sym;

            for(i32 i = 0; i < Vkey::_Count; i++) {
                if(g_vkeySdlKey[i].type != VkeyLink::Keyboard) continue;

                if(g_vkeySdlKey[i].key == sdlk) {
                    vkeyStatus[i] = true;
                    return;
                }
            }
        }

        else if(event.type == SDL_KEYUP) {
            const i32 sdlk = event.key.keysym.sym;

            for(i32 i = 0; i < Vkey::_Count; i++) {
                if(g_vkeySdlKey[i].type != VkeyLink::Keyboard) continue;

                if(g_vkeySdlKey[i].key == sdlk) {
                    vkeyStatus[i] = false;
                    return;
                }
            }
        }

        else if(event.type == SDL_MOUSEMOTION) {
            mouseX += event.motion.xrel;
            mouseY += event.motion.yrel;
            // TODO: plug in window resolution
            mouseX = clamp(mouseX, -800.f, 800.f);
            mouseY = clamp(mouseY, -450.f, 450.f);
            return;
        }

        else if(event.type == SDL_MOUSEBUTTONDOWN) {
            const u8 button = event.button.button;

            for(i32 i = 0; i < Vkey::_Count; i++) {
                if(g_vkeySdlKey[i].type != VkeyLink::Mouse) continue;

                if(g_vkeySdlKey[i].button == button) {
                    vkeyStatus[i] = true;
                    return;
                }
            }
        }

        else if(event.type == SDL_MOUSEBUTTONUP) {
            const u8 button = event.button.button;

            for(i32 i = 0; i < Vkey::_Count; i++) {
                if(g_vkeySdlKey[i].type != VkeyLink::Mouse) continue;

                if(g_vkeySdlKey[i].button == button) {
                    vkeyStatus[i] = false;
                    return;
                }
            }
        }
    }
};

static InputRecorder g_inputRecorder;

void inputRecorderHandleEvent(const SDL_Event& event)
{
    g_inputRecorder.handleEvent(event);
}

bool inputIsKeyPressed(Vkey::Enum vkey)
{
    return g_inputRecorder.vkeyStatus[vkey];
}

bool inputIsKeyReleased(Vkey::Enum vkey)
{
    return !g_inputRecorder.vkeyStatus[vkey];
}

void inputGetMousePos(f32* x, f32* y)
{
    *x = g_inputRecorder.mouseX;
    *y = g_inputRecorder.mouseY;
}
