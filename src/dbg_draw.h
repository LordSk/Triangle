#pragma once

#include "vector_math.h"

bool dbgDrawInit();
void dbgDrawDeinit();
void dbgDrawRender();

void dbgDrawRect(const Transform& tf, vec4 color);
void dbgDrawRectLine(const Transform& tf, vec4 color, f32 thick = 0.05f);
void dbgDrawSphere(const vec3& pos, f32 radius, vec4 color);
void dbgDrawLine(const vec3& p1, const vec3& p2, vec4 color, f32 thickness = 0.05f);
void dbgDrawOrthoFrustrum(const vec3& eye, const vec3& at, f32 near, f32 far,
                          f32 left, f32 right, f32 top, f32 bottom, vec4 color, f32 thick = 0.05f);
void dbgDrawOrthoFrustrumSolid(const vec3& eye, const vec3& at, f32 near, f32 far, f32 left,
                               f32 right, f32 top, f32 bottom, vec4 color, f32 thick = 0.05f);
