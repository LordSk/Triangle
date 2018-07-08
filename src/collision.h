#pragma once

#include "vector_math.h"

struct OrientedBoundingBox
{
    f32 angle;
    vec3 origin;
    vec3 size;
};

struct CircleBound
{
    vec3 center;
    f32 radius;
};

bool obbIntersectObb(const OrientedBoundingBox& obbA, const OrientedBoundingBox& obbB, void* out);
bool obbIntersectCb(const OrientedBoundingBox& obbA, const CircleBound& cbB, void* out);

void obbDbgDraw(const OrientedBoundingBox& obb, vec4 color);
void cbDbgDraw(const CircleBound& cb, vec4 color);

