#pragma once
#include "base.h"
#include <SDL2/SDL_events.h>

struct Vkey
{
    enum Enum {
        _Invalid = 0,
        Left,
        Right,
        Up,
        Down,
        Fire,
        _Count
    };
};

void inputRecorderHandleEvent(const SDL_Event& event);
void inputRecorderNewFrame();

bool inputIsKeyPressed(Vkey::Enum vkey);
bool inputIsKeyReleased(Vkey::Enum vkey);
void inputGetMousePos(f32* x, f32* y);
