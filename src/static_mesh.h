// simple hard coded meshes

#ifndef STATIC_MESH_H
#define STATIC_MESH_H

#include "base.h"
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

#endif

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
