#include "utils.h"

const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath)
{
    if(bx::open(_reader, _filePath)) {
        uint32_t size = (uint32_t)bx::getSize(_reader);
        const bgfx::Memory* mem = bgfx::alloc(size+1);
        bx::read(_reader, mem->data, size);
        bx::close(_reader);
        mem->data[mem->size-1] = '\0';
        return mem;
    }

    LOG("Failed to load %s.", _filePath);
    assert(0);
    return NULL;
}

bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const char* _name)
{
    char filePath[512];

    const char* ext = "???";

    switch(bgfx::getRendererType()) {
        case bgfx::RendererType::Noop:
        case bgfx::RendererType::Direct3D9:  ext = ".dx9";   break;
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12: ext = ".dx11";  break;
        case bgfx::RendererType::Gnm:        ext = ".pssl";  break;
        case bgfx::RendererType::Metal:      ext = ".metal"; break;
        case bgfx::RendererType::OpenGL:     ext = ".glsl";  break;
        case bgfx::RendererType::OpenGLES:   ext = ".essl";  break;
        case bgfx::RendererType::Vulkan:     ext = ".spirv"; break;

        case bgfx::RendererType::Count:
            assert(0);
            break;
    }

    bx::strCopy(filePath, BX_COUNTOF(filePath), _name);
    bx::strCat(filePath, BX_COUNTOF(filePath), ext);

    bgfx::ShaderHandle handle = bgfx::createShader(loadMem(_reader, filePath) );
    bgfx::setName(handle, filePath);

    return handle;
}

bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName)
{
    bgfx::ShaderHandle vsh = loadShader(_reader, _vsName);
    bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
    if (NULL != _fsName)
    {
        fsh = loadShader(_reader, _fsName);
    }

    return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
}

static u32 state32 = 0x123456;
u32 xorshift32()
{
    u32 x = state32;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state32 = x;
    return x;
}

f64 rand01()
{
    return (f64)xorshift32()/0xFFFFFFFF;
}

f64 rand1h()
{
    return rand01() * 2.0 - 1.0;
}

f64 randRange(f64 min, f64 max)
{
    return min + rand01() * (max - min);
}

void randSetSeed(u32 seed)
{
    state32 = seed;
}
