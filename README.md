# MemoryPool
Not thread safe, just a demo.

```
    char* p[1024];
    MemoryPool pool(4096);
    for(int i = 0; i < 1024; ++i)
    {
        p[i] = (char*)pool.alloc(4096);
    }
```
