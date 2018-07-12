#pragma once

#include "base.h"
#include <bgfx/bgfx.h>
#include <bx/file.h>

typedef u32 MeshHandle;
#define MESH_HANDLE_INVALID 0

MeshHandle meshLoad(const char* filePath, bx::FileReaderI* reader);
void meshUnload(MeshHandle meshHnd);
void meshSubmit(MeshHandle mesh, bgfx::ViewId _id,
                bgfx::ProgramHandle _program, const float* _mtx, u64 _state);
void meshSubmitGroup(MeshHandle meshHnd, i32 groupId, bgfx::ViewId _id, bgfx::ProgramHandle _program,
                     const float* _mtx, u64 _state);
