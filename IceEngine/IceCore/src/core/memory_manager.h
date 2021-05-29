
#ifndef CORE_MEMORY_MANAGER_H
#define CORE_MEMORY_MANAGER_H 1

#include "defines.h"

enum IceMemoryTypeBits
{
  IMEM_TYPE_UNKNOWN,
  IMEM_TYPE_BUFFER,
  IMEM_TYPE_COUNT
};
typedef IceFlag IceMemoryTypeFlag;

class MemoryManager
{
public:
  struct MemoryStatistics
  {
    u64 totalMemoryAllocated;
    u64 categoryAllocations[IMEM_TYPE_COUNT];
  };

private:
  MemoryStatistics stats{};
  static MemoryManager* instance;

  //=================================================================================================
  // Functions
  //=================================================================================================
public:
  static void Initialize();
  static void Shutdown();
  static MemoryManager* GetInstance();
  MemoryStatistics* GetStats();

  static void* Allocate(u32 size, IceMemoryTypeFlag flag);
  static void Free(void* data, u32 size, IceMemoryTypeFlag flag);
  static void Zero(void* data, u32 size);

  static void PrintStats();
};

#endif // !CORE_MEMORY_MANAGER_H
