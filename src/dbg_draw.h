#pragma once

#include "vector_math.h"

bool dbgDrawInit();
void dbgDrawDeinit();
void dbgDrawRender();

void dbgDrawRect(const Transform& tf, vec4 color);
void dbgDrawSphere(const vec3& pos, f32 radius, vec4 color);
void dbgDrawLine(const vec3& p1, const vec3& p2, vec4 color, f32 thickness = 0.05f);
