#include "mesh_load.h"

#include <vector>
#include <ib-compress/indexbufferdecompression.h>
#include <bx/file.h>
#include <unordered_map>

struct Aabb
{
    float m_min[3];
    float m_max[3];
};

struct Obb
{
    float m_mtx[16];
};

struct Sphere
{
    float m_center[3];
    float m_radius;
};

struct Primitive
{
    uint32_t m_startIndex;
    uint32_t m_numIndices;
    uint32_t m_startVertex;
    uint32_t m_numVertices;

    Sphere m_sphere;
    Aabb m_aabb;
    Obb m_obb;
};

typedef std::vector<Primitive> PrimitiveArray;

struct Group
{
    Group()
    {
        reset();
    }

    void reset()
    {
        m_vbh.idx = bgfx::kInvalidHandle;
        m_ibh.idx = bgfx::kInvalidHandle;
        m_prims.clear();
    }

    bgfx::VertexBufferHandle m_vbh;
    bgfx::IndexBufferHandle m_ibh;
    Sphere m_sphere;
    Aabb m_aabb;
    Obb m_obb;
    PrimitiveArray m_prims;
};

namespace bgfx
{
    int32_t read(bx::ReaderI* _reader, bgfx::VertexDecl& _decl, bx::Error* _err = NULL);
}


struct Mesh
{
    bool load(const char* filePath, bx::FileReaderI* reader)
    {
        if(!bx::open(reader, filePath)) {
            return false;
        }
        bx::ReaderSeekerI* _reader = reader;

#define BGFX_CHUNK_MAGIC_VB  BX_MAKEFOURCC('V', 'B', ' ', 0x1)
#define BGFX_CHUNK_MAGIC_IB  BX_MAKEFOURCC('I', 'B', ' ', 0x0)
#define BGFX_CHUNK_MAGIC_IBC BX_MAKEFOURCC('I', 'B', 'C', 0x0)
#define BGFX_CHUNK_MAGIC_PRI BX_MAKEFOURCC('P', 'R', 'I', 0x0)

        Group group;

        bx::AllocatorI* allocator = getDefaultAllocator();

        uint32_t chunk;
        bx::Error err;
        while(4 == bx::read(_reader, chunk, &err) && err.isOk()) {
            switch(chunk) {
                case BGFX_CHUNK_MAGIC_VB: {
                    bx::read(_reader, group.m_sphere);
                    bx::read(_reader, group.m_aabb);
                    bx::read(_reader, group.m_obb);

                    bgfx::read(_reader, m_decl); // only works with bgfx::read for some reason?

                    uint16_t stride = m_decl.getStride();

                    uint16_t numVertices;
                    bx::read(_reader, numVertices);
                    const bgfx::Memory* mem = bgfx::alloc(numVertices*stride);
                    bx::read(_reader, mem->data, mem->size);

                    group.m_vbh = bgfx::createVertexBuffer(mem, m_decl);
                } break;

                case BGFX_CHUNK_MAGIC_IB: {
                    uint32_t numIndices;
                    bx::read(_reader, numIndices);
                    const bgfx::Memory* mem = bgfx::alloc(numIndices*2);
                    bx::read(_reader, mem->data, mem->size);
                    group.m_ibh = bgfx::createIndexBuffer(mem);
                } break;

                case BGFX_CHUNK_MAGIC_IBC: {
                    uint32_t numIndices;
                    bx::read(_reader, numIndices);

                    const bgfx::Memory* mem = bgfx::alloc(numIndices*2);

                    uint32_t compressedSize;
                    bx::read(_reader, compressedSize);

                    void* compressedIndices = BX_ALLOC(allocator, compressedSize);

                    bx::read(_reader, compressedIndices, compressedSize);

                    ReadBitstream rbs( (const uint8_t*)compressedIndices, compressedSize);
                    DecompressIndexBuffer( (uint16_t*)mem->data, numIndices / 3, rbs);

                    BX_FREE(allocator, compressedIndices);

                    group.m_ibh = bgfx::createIndexBuffer(mem);
                } break;

                case BGFX_CHUNK_MAGIC_PRI: {
                    uint16_t len;
                    bx::read(_reader, len);

                    std::string material;
                    material.resize(len);
                    bx::read(_reader, const_cast<char*>(material.c_str()), len);

                    uint16_t num;
                    bx::read(_reader, num);

                    for(uint32_t ii = 0; ii < num; ++ii) {
                        bx::read(_reader, len);

                        std::string name;
                        name.resize(len);
                        bx::read(_reader, const_cast<char*>(name.c_str() ), len);

                        Primitive prim;
                        bx::read(_reader, prim.m_startIndex);
                        bx::read(_reader, prim.m_numIndices);
                        bx::read(_reader, prim.m_startVertex);
                        bx::read(_reader, prim.m_numVertices);
                        bx::read(_reader, prim.m_sphere);
                        bx::read(_reader, prim.m_aabb);
                        bx::read(_reader, prim.m_obb);

                        group.m_prims.push_back(prim);
                    }

                    m_groups.push_back(group);
                    group.reset();
                } break;

                default:
                    LOG("%08x at %lld", chunk, bx::skip(_reader, 0) );
                    break;
            }
        }

        bx::close(reader);
        return true;
    }

    void unload()
    {
        for(GroupArray::const_iterator it = m_groups.begin(), itEnd = m_groups.end(); it != itEnd; ++it) {
            const Group& group = *it;
            bgfx::destroy(group.m_vbh);

            if(bgfx::isValid(group.m_ibh)) {
                bgfx::destroy(group.m_ibh);
            }
        }
        m_groups.clear();
    }

    void submit(bgfx::ViewId _id, bgfx::ProgramHandle _program, const float* _mtx, uint64_t _state) const
    {
        if(BGFX_STATE_MASK == _state) {
            _state = 0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
                ;
        }

        bgfx::setTransform(_mtx);
        bgfx::setState(_state);

        for(GroupArray::const_iterator it = m_groups.begin(), itEnd = m_groups.end(); it != itEnd; ++it) {
            const Group& group = *it;

            bgfx::setIndexBuffer(group.m_ibh);
            bgfx::setVertexBuffer(0, group.m_vbh);
            bgfx::submit(_id, _program, 0, it != itEnd-1);
        }
    }

    /*void submit(const MeshState*const* _state, uint8_t _numPasses, const float* _mtx, uint16_t _numMatrices) const
    {
        uint32_t cached = bgfx::setTransform(_mtx, _numMatrices);

        for (uint32_t pass = 0; pass < _numPasses; ++pass)
        {
            bgfx::setTransform(cached, _numMatrices);

            const MeshState& state = *_state[pass];
            bgfx::setState(state.m_state);

            for (uint8_t tex = 0; tex < state.m_numTextures; ++tex)
            {
                const MeshState::Texture& texture = state.m_textures[tex];
                bgfx::setTexture(texture.m_stage
                        , texture.m_sampler
                        , texture.m_texture
                        , texture.m_flags
                        );
            }

            for (GroupArray::const_iterator it = m_groups.begin(), itEnd = m_groups.end(); it != itEnd; ++it)
            {
                const Group& group = *it;

                bgfx::setIndexBuffer(group.m_ibh);
                bgfx::setVertexBuffer(0, group.m_vbh);
                bgfx::submit(state.m_viewId, state.m_program, 0, it != itEnd-1);
            }
        }
    }*/

    bgfx::VertexDecl m_decl;
    typedef std::vector<Group> GroupArray;
    GroupArray m_groups;
};

struct MeshHolder
{
    std::unordered_map<MeshHandle, Mesh> meshMap;

    MeshHandle addMesh(const char* filePath, Mesh mesh)
    {
        MeshHandle h = std::hash<std::string>{}(std::string(filePath));
        meshMap[h] = mesh;
        return h;
    }
};

static MeshHolder g_meshHolder;

MeshHandle meshLoad(const char* filePath, bx::FileReaderI* reader)
{
    Mesh mesh;
    if(!mesh.load(filePath, reader)) {
        return MESH_HANDLE_INVALID;
    }

    return g_meshHolder.addMesh(filePath, mesh);
}

void meshUnload(MeshHandle meshHnd)
{
    if(meshHnd == MESH_HANDLE_INVALID) {
        return;
    }

    assert(g_meshHolder.meshMap.find(meshHnd) != g_meshHolder.meshMap.end());
    Mesh& mesh = g_meshHolder.meshMap[meshHnd];
    mesh.unload();
    g_meshHolder.meshMap.erase(meshHnd);
}

void meshSubmit(MeshHandle meshHnd, bgfx::ViewId _id, bgfx::ProgramHandle _program,
                const float* _mtx, u64 _state)
{
    if(meshHnd == MESH_HANDLE_INVALID) {
        return;
    }

    assert(g_meshHolder.meshMap.find(meshHnd) != g_meshHolder.meshMap.end());
    Mesh& mesh = g_meshHolder.meshMap[meshHnd];
    mesh.submit(_id, _program, _mtx, _state);
}
