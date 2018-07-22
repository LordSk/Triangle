#pragma once

#include "base.h"
#include <vector>
#include <bgfx/bgfx.h>
#include <bx/file.h>

// TODO: replace this with our own array struct
template<typename T>
struct Array: public std::vector<T>
{
    inline T& push(T elem) {
        push_back(elem);
        return *(end()-1);
    }

    inline void pop() {
        std::vector<T>::pop_back();
    }

    inline void insert(i32 where, T elem) {
        std::vector<T>::insert(begin() + where, elem);
    }

    inline i32 count() const {
        return size();
    }

    inline T& last() {
        return back();
    }
};


/**
 *  List
 *  - growable array
 *  - can't access all elements sequentially (for now) due to buckets
 *  - pointers are not invalidated on resize (grow)
 */
template<typename T, i32 BUCKET_ELT_COUNT=1024>
struct List
{
    struct Bucket
    {
        T* buffer;
        u32 count;
        u32 capacity;
    };

    T stackData[BUCKET_ELT_COUNT];
    Array<Bucket> buckets;

    List() {
        Bucket buck;
        buck.capacity = BUCKET_ELT_COUNT;
        buck.buffer = stackData;
        buck.count = 0;
        buckets.push(buck);
    }

    ~List() {
        const i32 bucketCount = buckets.count();
        for(i32 i = 1; i < bucketCount; ++i) {
            free(buckets[i].buffer);
        }
        buckets.clear();
    }

    inline void _allocNewBucket() {
        const i32 shift = mmax(0, buckets.count()-1);
        Bucket buck;
        buck.capacity = BUCKET_ELT_COUNT << shift;
        buck.buffer = (T*)malloc(sizeof(T) * buck.capacity);
        buck.count = 0;
        buckets.push(buck);
    }

    inline T& push(T elem) {
        Bucket* buck = &buckets[buckets.count()-1];
        if(buck->count+1 >= buck->capacity) {
            _allocNewBucket();
        }
        buck = &buckets[buckets.count()-1];
        return (buck->buffer[buck->count++] = elem);
    }

    inline void clear() {
        // clear all but the first
        const i32 bucketCount = buckets.count();
        for(i32 i = 1; i < bucketCount; ++i) {
            free(buckets[i].buffer);
        }
        Bucket first = buckets[0];
        first.count = 0;
        buckets.clear();
        buckets.push(first);
    }

    inline T& operator[](const i32 id) {
        i32 bucketStart = 0;
        i32 bucketEnd = BUCKET_ELT_COUNT;
        i32 bucketId = 0;

        while(id >= bucketEnd) {
            bucketStart = bucketEnd;
            bucketEnd *= 2;
            bucketId++;
        }

        assert(bucketId < buckets.count());
        const i32 bbid = id - bucketStart;
        assert(bbid < buckets[bucketId].count);
        return buckets[bucketId].buffer[bbid];
    }
};

const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath);
bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const char* _name);
bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName);

u32 xorshift32();
f64 rand01();
f64 rand1h();
f64 randRange(f64 min, f64 max);
void randSetSeed(u32 seed);

