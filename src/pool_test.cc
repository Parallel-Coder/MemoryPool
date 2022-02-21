#include "MemoryPool.h"

using pinkx::MemoryPool;

int main()
{
    char* p[1024];
    MemoryPool pool(4096);
    for(int i = 0; i < 1024; ++i)
    {
        p[i] = (char*)pool.alloc(4096);
    }
}