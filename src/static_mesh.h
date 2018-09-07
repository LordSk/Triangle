// simple hard coded meshes

#ifndef STATIC_MESH_H
#define STATIC_MESH_H

#include "base.h"
#include "vector_math.h"
#include "renderer.h"

inline void makeSphere(PosColorNormalVertex* vertData, const i32 buffCount, i32* out_count,
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
            vertData[vertCount++] = PosColorNormalVertex{ s1.x, s1.y, s1.z };
            vertData[vertCount++] = PosColorNormalVertex{ r0s0.x, r0s0.y, r0s0.z };
            vertData[vertCount++] = PosColorNormalVertex{ s0.x, s0.y, s0.z };

            vertData[vertCount++] = PosColorNormalVertex{ s1.x, s1.y, s1.z };
            vertData[vertCount++] = PosColorNormalVertex{ r0s1.x, r0s1.y, r0s1.z };
            vertData[vertCount++] = PosColorNormalVertex{ r0s0.x, r0s0.y, r0s0.z };
        }

        // last segment from 0 to end
        const vec3 r0s0 = ringPoints[(r-1) * segCount + segCount - 1];
        const vec3 r0s1 = ringPoints[(r-1) * segCount];
        const vec3 s0 = ringPoints[r * segCount + segCount - 1];
        const vec3 s1 = ringPoints[r * segCount];

        assert(vertCount + 6 <= buffCount);
        vertData[vertCount++] = PosColorNormalVertex{ s1.x, s1.y, s1.z };
        vertData[vertCount++] = PosColorNormalVertex{ r0s0.x, r0s0.y, r0s0.z };
        vertData[vertCount++] = PosColorNormalVertex{ s0.x, s0.y, s0.z };

        vertData[vertCount++] = PosColorNormalVertex{ s1.x, s1.y, s1.z };
        vertData[vertCount++] = PosColorNormalVertex{ r0s1.x, r0s1.y, r0s1.z };
        vertData[vertCount++] = PosColorNormalVertex{ r0s0.x, r0s0.y, r0s0.z };
    }

    *out_count = vertCount;
}

#endif
