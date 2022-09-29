
#ifndef ICE_PLATFORM_MEMORY_POOL_H_
#define ICE_PLATFORM_MEMORY_POOL_H_

#include "defines.h"

#include "core/platform/platform.h"

namespace Ice {

class MemoryPool
{
private:
  u32 poolSize = 0;
  void* data = nullptr;

public:
  // _size : Size of the pool in bytes
  MemoryPool(u64 _size)
  {
    poolSize = _size;
    data = Ice::MemoryAllocate(_size);
  }

  ~MemoryPool()
  {
    Ice::MemoryFree(data);
  }



};

} // namespace Ice

#endif // !ICE_PLATFORM_MEMORY_POOL_H_
