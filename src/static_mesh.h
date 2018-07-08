// simple hard coded meshes

#ifndef STATIC_MESH_H
#define STATIC_MESH_H

#include "base.h"
#include "vector_math.h"
#include <bgfx/bgfx.h>

struct PosColorVertex
{
    f32 x, y, z;
    u32 color_abgr;
    f32 nx, ny, nz;

    static void init()
    {
        ms_decl
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
            .add(bgfx::Attrib::Normal,   3, bgfx::AttribType::Float)
            .end();
    }

    static bgfx::VertexDecl ms_decl;
};

inline void makeSphere(PosColorVertex* vertData, const i32 buffCount, i32* out_count,
                       f64 radius = 1.0, const i32 segCount = 32, const i32 ringCount = 24)
{
    const i32 pointCount = segCount * ringCount * 2;

    assert(pointCount < 4096);
    vec3 ringPoints[4096];
    i32 ringPointCount = 0;

    const f64 segStep = bx::kPi2 / segCount;
    const i32 ringCountHalf = ringCount/2;
    const f64 ringStep = radius / ringCountHalf;

    for(i32 r = 0; r < ringCountHalf; r++) {
        for(i32 s = 0; s < segCount; s++) {
            f32 zr = bx::cos(bx::asin(1.0 - (f64)r/ringCountHalf));
            ringPoints[ringPointCount++] =
                vec3{
                   f32(bx::cos(segStep * s) * zr * radius),
                   f32(bx::sin(segStep * s) * zr * radius),
                   f32(-radius + ringStep * r)
                };
        }
    }

    for(i32 r = ringCountHalf-1; r >= 0; r--) {
        for(i32 s = 0; s < segCount; s++) {
            f32 zr = bx::cos(bx::asin(1.0 - (f64)r/ringCountHalf));
            ringPoints[ringPointCount++] =
                vec3{
                   f32(bx::cos(segStep * s) * zr * radius),
                   f32(bx::sin(segStep * s) * zr * radius),
                   f32(radius - ringStep * r)
                };
        }
    }

    i32 vertCount = 0;

    // TODO: normals
    for(i32 r = 1; r < ringCount; r++) {
        for(i32 s = 1; s < segCount; s++) {
            const vec3 r0s0 = ringPoints[(r-1) * segCount + s - 1];
            const vec3 r0s1 = ringPoints[(r-1) * segCount + s];
            const vec3 s0 = ringPoints[r * segCount + s - 1];
            const vec3 s1 = ringPoints[r * segCount + s];

            assert(vertCount + 6 <= buffCount);
            vertData[vertCount++] = PosColorVertex{ s1.x, s1.y, s1.z };
            vertData[vertCount++] = PosColorVertex{ r0s0.x, r0s0.y, r0s0.z };
            vertData[vertCount++] = PosColorVertex{ s0.x, s0.y, s0.z };

            vertData[vertCount++] = PosColorVertex{ s1.x, s1.y, s1.z };
            vertData[vertCount++] = PosColorVertex{ r0s1.x, r0s1.y, r0s1.z };
            vertData[vertCount++] = PosColorVertex{ r0s0.x, r0s0.y, r0s0.z };
        }

        // last segment from 0 to end
        const vec3 r0s0 = ringPoints[(r-1) * segCount + segCount - 1];
        const vec3 r0s1 = ringPoints[(r-1) * segCount];
        const vec3 s0 = ringPoints[r * segCount + segCount - 1];
        const vec3 s1 = ringPoints[r * segCount];

        assert(vertCount + 6 <= buffCount);
        vertData[vertCount++] = PosColorVertex{ s1.x, s1.y, s1.z };
        vertData[vertCount++] = PosColorVertex{ r0s0.x, r0s0.y, r0s0.z };
        vertData[vertCount++] = PosColorVertex{ s0.x, s0.y, s0.z };

        vertData[vertCount++] = PosColorVertex{ s1.x, s1.y, s1.z };
        vertData[vertCount++] = PosColorVertex{ r0s1.x, r0s1.y, r0s1.z };
        vertData[vertCount++] = PosColorVertex{ r0s0.x, r0s0.y, r0s0.z };
    }

    *out_count = vertCount;
}

#endif

#ifdef STATIC_MESH_DATA

static PosColorVertex s_originVertData[] =
{
    {0, 0, 0, 0xffff0000 },
    {10, 0, 0, 0xffff0000 },

    {0, 0, 0, 0xff00ff00 },
    {0, 10, 0, 0xff00ff00 },

    {0, 0, 0, 0xff0000ff },
    {0, 0, 10, 0xff0000ff },
};

static PosColorVertex s_cubeRainbowVertData[] =
{
    // left
    {-0.5f, -0.5f, -0.5f, 0xffffff00, -1.0f, 0.0f, 0.0f },
    {-0.5f,  0.5f,  0.5f, 0xff000000, -1.0f, 0.0f, 0.0f },
    {-0.5f, -0.5f,  0.5f, 0xff00ff00, -1.0f, 0.0f, 0.0f },
    {-0.5f, -0.5f, -0.5f, 0xffffff00, -1.0f, 0.0f, 0.0f },
    {-0.5f,  0.5f, -0.5f, 0xffff00ff, -1.0f, 0.0f, 0.0f },
    {-0.5f,  0.5f,  0.5f, 0xff000000, -1.0f, 0.0f, 0.0f },


    // right
    { 0.5f,  0.5f,  0.5f, 0xff0000ff, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f, -0.5f, 0xffff00ff, 1.0f, 0.0f, 0.0f },
    { 0.5f, -0.5f, -0.5f, 0xffffffff, 1.0f, 0.0f, 0.0f },
    { 0.5f, -0.5f, -0.5f, 0xffffffff, 1.0f, 0.0f, 0.0f },
    { 0.5f, -0.5f,  0.5f, 0xff00ffff, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  0.5f, 0xff0000ff, 1.0f, 0.0f, 0.0f },


    // bottom
    { 0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 0.0f, -1.0f },
    {-0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 0.0f, -1.0f },
    {-0.5f, -0.5f, -0.5f, 0xffffff00, 0.0f, 0.0f, -1.0f },
    { 0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 0.0f, -1.0f },
    {-0.5f, -0.5f, -0.5f, 0xffffff00, 0.0f, 0.0f, -1.0f },
    { 0.5f, -0.5f, -0.5f, 0xffffffff, 0.0f, 0.0f, -1.0f },


    // top
    {-0.5f,  0.5f,  0.5f, 0xff000000, 0.0f, 0.0f, 1.0f },
    { 0.5f, -0.5f,  0.5f, 0xff00ffff, 0.0f, 0.0f, 1.0f },
    {-0.5f, -0.5f,  0.5f, 0xff00ff00, 0.0f, 0.0f, 1.0f },
    { 0.5f,  0.5f,  0.5f, 0xff0000ff, 0.0f, 0.0f, 1.0f },
    { 0.5f, -0.5f,  0.5f, 0xff00ffff, 0.0f, 0.0f, 1.0f },
    {-0.5f,  0.5f,  0.5f, 0xff000000, 0.0f, 0.0f, 1.0f },


    // front
    { 0.5f, -0.5f,  0.5f, 0xff00ffff, 0.0f, -1.0f, 0.0f },
    { 0.5f, -0.5f, -0.5f, 0xffffffff, 0.0f, -1.0f, 0.0f },
    {-0.5f, -0.5f, -0.5f, 0xffffff00, 0.0f, -1.0f, 0.0f },
    { 0.5f, -0.5f,  0.5f, 0xff00ffff, 0.0f, -1.0f, 0.0f },
    {-0.5f, -0.5f, -0.5f, 0xffffff00, 0.0f, -1.0f, 0.0f },
    {-0.5f, -0.5f,  0.5f, 0xff00ff00, 0.0f, -1.0f, 0.0f },


    // back
    { 0.5f,  0.5f,  0.5f, 0xff0000ff, 0.0f, 1.0f, 0.0f },
    {-0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 1.0f, 0.0f },
    { 0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 1.0f, 0.0f },
    { 0.5f,  0.5f,  0.5f, 0xff0000ff, 0.0f, 1.0f, 0.0f },
    {-0.5f,  0.5f,  0.5f, 0xff000000, 0.0f, 1.0f, 0.0f },
    {-0.5f,  0.5f, -0.5f, 0xffff00ff, 0.0f, 1.0f, 0.0f },
};

static PosColorVertex s_gridLinesVertData[400];
#endif
