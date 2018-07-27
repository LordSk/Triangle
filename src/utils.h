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

template<typename T>
struct ArraySparse
{
    Array<T> _data;
    Array<i32> _dataEltId;
    Array<i32> _eltDataId;

    ArraySparse() {
        _data.reserve(64);
        _dataEltId.reserve(64);

        _eltDataId.resize(64);
        const i32 eltDataIdCount = _eltDataId.count();
        for(i32 i = 0; i < eltDataIdCount; i++) {
            _eltDataId[i] = -1;
        }
    }

    inline void _doubleIdListSize() {
        const i32 oldCount = _eltDataId.count();
        _eltDataId.resize(_eltDataId.count() * 2);
        const i32 newCount = _eltDataId.count();
        for(i32 i = oldCount; i < newCount; i++) {
            _eltDataId[i] = -1;
        }
    }

    inline i32 findFirstFreeId() const {
        const i32 eltDataIdCount = _eltDataId.count();
        for(i32 i = 0; i < eltDataIdCount; i++) {
            if(_eltDataId[i] == -1) {
                return i;
            }
        }
        return -1;
    }

    inline T& push(T elem) {
        i32 id = findFirstFreeId();
        if(id == -1) { // resize eltDataId array
            id = _eltDataId.count();
            _doubleIdListSize();
        }
        return emplace(id, elem);
    }

    inline T& emplace(const i32 id, T elem) {
        if(id >= _eltDataId.count()) {
            _doubleIdListSize();
            assert(id < _eltDataId.count());
        }
        if(_eltDataId[id] == -1) {
            _eltDataId[id] = _data.count();
            _dataEltId.push(id);
            return _data.push(elem);
        }
        return (_data[_eltDataId[id]] = elem);
    }

    inline void removeById(const i32 id) {
        assert(id < _eltDataId.count());
        if(_eltDataId[id] == -1) return;
        const i32 dataId = _eltDataId[id];
        const i32 swapEltId = _dataEltId.last();
        _eltDataId[swapEltId] = dataId;
        _dataEltId[dataId] = swapEltId;
        swap(&_data[dataId], &_data.last());
        _data.pop();
        _dataEltId.pop();
        _eltDataId[id] = -1;
    }

    inline void removeByElt(const T& elt) {
        assert(&elt >= _data.data() && &elt <= &_data.last());
        removeById(&elt - _data.data());
    }

    inline i32 count() const {
        return _data.count();
    }

    inline T* data() {
        return _data.data();
    }

    inline T& operator[](const i32 id) {
        assert(id < _eltDataId.count());
        assert(_eltDataId[id] != -1);
        return _data[_eltDataId[id]];
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

