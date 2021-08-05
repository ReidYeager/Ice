
#ifndef ICE_CORE_MEMORY_MANAGER_H_
#define ICE_CORE_MEMORY_MANAGER_H_

#include "defines.h"

enum IceMemoryTypeBits
{
  IMEM_TYPE_UNKNOWN,
  IMEM_TYPE_BUFFER,
  IMEM_TYPE_COUNT
};
typedef IceFlag IceMemoryTypeFlag;

class IceMemoryManager
{
public:
  struct MemoryStatistics
  {
    u64 totalMemoryAllocated;
    u64 categoryAllocations[IMEM_TYPE_COUNT];
  };

private:
  MemoryStatistics stats{};

  //=================================================================================================
  // Functions
  //=================================================================================================
public:
  void Initialize();
  void Shutdown();
  MemoryStatistics* GetStats();

  void* Allocate(u32 size, IceMemoryTypeFlag flag);
  void Free(void* data, u32 size, IceMemoryTypeFlag flag);
  void Zero(void* data, u32 size);
  void Copy(void* dst, void* src, u32 size);

  void PrintStats();
};

extern IceMemoryManager MemoryManager;

#endif // !CORE_MEMORY_MANAGER_H
