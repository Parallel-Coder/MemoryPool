#pragma once
#include <assert.h>
#include <algorithm>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>



namespace pinkx {
#define DEFAULT_SMALL_SIZE  4096   

typedef unsigned char uchar; 

struct PMemoryPool;
class MemoryPool;

// This is a small data of pool 
struct PMemoryData {
    
    uchar*          last;       // Begin of small memory
    uchar*          end;        // End of small memory
    PMemoryPool*    next;
    int             fail;       // Memory alloc fail and if fail more than 4 times, it will be useless
    size_t          size;

    PMemoryData():last(nullptr), end(nullptr), fail(0) {
        // Do nothing
    }

};



struct PLargeData {
    PLargeData*     next;       // Next
    uchar*          memory;     // Memory

    PLargeData():next(nullptr), memory(nullptr) {
        // Do nothing
    }
};



class PMemoryPool {
private:
    uchar*          alloc_small(size_t size);           // Alloc small block           
    uchar*          alloc_large(size_t size);           // Alloc large block
    uchar*          palloc_block(size_t size);          // Create a new block, and alloc.
    size_t          max;
    PMemoryData     small_data;
    PLargeData*     large_data;

    PMemoryPool*    current;                         
public:
    friend class MemoryPool;
    uchar*  alloc(size_t size);
    uchar*  calloc(size_t size);
    void    reset();
    void    pfree(void* p);

    template<typename T>
    T* alloc(size_t size) {
        return reinterpret_cast<T*>(alloc(size));
    }
};


class MemoryPool {
private:
    PMemoryPool* pool_;
public:
    MemoryPool(size_t size);
    ~MemoryPool();

    uchar* alloc(size_t size);
};

class PoolFactor {
public:
    static PMemoryPool* CreatePool();
    static PMemoryPool* CreatePool(size_t size);
    static void         DestoryPool(pinkx::PMemoryPool* pool);
};
}