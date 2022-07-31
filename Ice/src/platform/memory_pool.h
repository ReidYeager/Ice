
#ifndef ICE_PLATFORM_MEMORY_POOL_H_
#define ICE_PLATFORM_MEMORY_POOL_H_

#include "defines.h"

#include "platform/platform.h"

namespace Ice {

class MemoryPool
{
private:
  u32 elementSize = 0; // The size of one element
  u32 elementCount = 0;
  void* data = nullptr;

public:
  // _size : Size of the pool in bytes
  MemoryPool(u32 _size, u32 _count)
  {
    elementSize = _size;
    elementCount = _count;
    data = Ice::MemoryAllocate(_size * _count);
  }

  ~MemoryPool()
  {
    Ice::MemoryFree(data);
  }

  inline void* operator[](int _index)
  {
    return (char*)data + (_index * elementSize);
  }

  template <typename T>
  T* AsArray(u32* _count = nullptr)
  {
    if (_count != nullptr)
      *_count = elementCount;

    return (T*)data;
  }

};

} // namespace Ice

#endif // !ICE_PLATFORM_MEMORY_POOL_H_
