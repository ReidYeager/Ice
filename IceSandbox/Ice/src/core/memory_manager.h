
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

// A foundation to handle memory categorized by use
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
  // Ensures the memory statistics are 0
  void Initialize();
  // Logs amount of unfreed memory
  // (Used primarily to help track leaks)
  void Shutdown();
  MemoryStatistics* GetStats();

  // Allocates memory under the input category
  void* Allocate(u32 size, IceMemoryTypeFlag flag);
  // Frees memory under the input category
  void Free(void* data, u32 size, IceMemoryTypeFlag flag);
  // Sets a block of memory to 0
  void Zero(void* data, u32 size);
  // Copies the data in src to dst
  void Copy(void* dst, void* src, u32 size);

  // Logs the current amount of memory allocated for each category
  void PrintStats();
};

extern IceMemoryManager MemoryManager;

#endif // !CORE_MEMORY_MANAGER_H
