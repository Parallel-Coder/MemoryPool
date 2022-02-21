#include "MemoryPool.h"

pinkx::uchar* pinkx::PMemoryPool::alloc(size_t size) {
  if (size < max) {
    return alloc_small(size);
  } else {
    return alloc_large(size);
  }
}

pinkx::uchar* pinkx::PMemoryPool::calloc(size_t size) {
  pinkx::uchar* ret = alloc(size);
  if (ret == nullptr) return nullptr;
  memset(ret, 0, size);
  return ret;
}

void pinkx::PMemoryPool::reset() {
  PMemoryPool* p = this;
  PLargeData* large = p->large_data;
  for (; p; p = p->small_data.next) {
    PMemoryData& d = p->small_data;
    d.end = d.last;
  }

  for (; large; large = large->next) {
    delete[](uchar*) large->memory;
    large->memory = nullptr;
  }
}

// Free large memory
void pinkx::PMemoryPool::pfree(void* p) {
  PLargeData* l;
  for (l = large_data; l; l = l->next) {
    if (l->memory == p) {
      delete[](uchar*) l->memory;
      l->memory = nullptr;
    }
  }
}

pinkx::uchar* pinkx::PMemoryPool::alloc_small(size_t size) {
  PMemoryData* small = &small_data;
  size_t remain_size = small->end - small->last;
  PMemoryPool* p = current;

  do {
    uchar* m = p->small_data.last;
    if (static_cast<size_t>((p->small_data.end - m)) >= size) {
      p->small_data.last = m + size;
      return m;
    }
    p = p->small_data.next;
  } while (p);

  return palloc_block(size);
}

pinkx::uchar* pinkx::PMemoryPool::palloc_block(size_t size) {
  uchar* m;
  PMemoryPool *new_p, *p;
  size_t alloc_size = (uchar*)small_data.end - (uchar*)this;
  m = new uchar[alloc_size];
  // malloc :(
  if (m == nullptr) return nullptr;

  new_p = reinterpret_cast<PMemoryPool*>(m);
  new_p->large_data = nullptr;
  new_p->small_data.next = nullptr;
  new_p->small_data.fail = 0;
  new_p->small_data.last = m + sizeof(PMemoryPool) + size;
  new_p->small_data.end = m + alloc_size;

  m += sizeof(PMemoryPool);

  for (p = current; p->small_data.next; p = p->small_data.next) {
    if (p->small_data.fail++ > 4) {
      current = p->small_data.next;
    }
  }

  p->small_data.next = new_p;

  return m;
}

pinkx::uchar* pinkx::PMemoryPool::alloc_large(size_t size) {
  uchar* m;
  PLargeData* l;

  m = reinterpret_cast<uchar*>(new uchar[size]);
  if (m == nullptr) return nullptr;

  int n = 0;
  for (l = large_data; l; l = l->next) {
    if (l->memory == nullptr) {
      l->memory = m;
      return m;
    }
    if (n++ > 3) {
      break;
    }
  }

  l = reinterpret_cast<PLargeData*>(alloc_small(sizeof(PLargeData)));
  if (l == nullptr) {
    delete[] m;
    return nullptr;
  }

  l->memory = m;
  l->next = large_data;
  large_data = l;

  return m;
}

pinkx::MemoryPool::MemoryPool(size_t size) {
  PMemoryPool* p;
  size_t alloc_size = size;

  p = reinterpret_cast<PMemoryPool*>(new uchar[alloc_size]);
  assert(p);
  p->small_data.last = (reinterpret_cast<uchar*>(p) + sizeof(PMemoryPool));
  p->small_data.end = (reinterpret_cast<uchar*>(p) + alloc_size);
  p->small_data.next = nullptr;
  p->small_data.fail = 0;
  p->max = size;
  p->current = p;
  p->large_data = nullptr;

  pool_ = p;
}

pinkx::MemoryPool::~MemoryPool() {
  auto pool = pool_;
  if (pool == nullptr) return;
  for (PLargeData* l = pool->large_data; l; l = l->next) {
    // Delete large
    delete[] l->memory;
  }
  for (PMemoryPool* p = pool; p;) {
    PMemoryPool* next = p->small_data.next;
    delete[] reinterpret_cast<uchar*>(p);
    p = next;
  }
}

pinkx::uchar* pinkx::MemoryPool::alloc(size_t size) {
  uchar* ret = pool_->alloc(size);
  return ret;
}
