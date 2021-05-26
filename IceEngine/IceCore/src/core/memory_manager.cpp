
#include "core/memory_manager.h"
#include "platform/platform.h"

const char* IceMemoryTypeStrings[IMEM_COUNT] = {
  "IMEM_UNKNOWN",
  "IMEM_ARRAY",
  "IMEM_TEXTURE",
  "IMEM_BUFFER"
};

struct IceMemoryStats
{
  u64 totalAllocationAmount;
  u64 typeAllocationAmounts[IMEM_COUNT];
  //std::vector<void*> allocations[IMEM_COUNT]; // Not sure if this is a good idea
};
IceMemoryStats stats;

void IMemInitialize()
{
  PlatformZeroMem(&stats, sizeof(IceMemoryStats));
}

void IMemShutdown()
{
  IMemLogStats();
  // TODO : Free any remaining allocated memory
}

void* IMemAllocate(u64 _size, IceMemoryTypeFlag _type)
{
  stats.typeAllocationAmounts[_type] += _size;
  stats.totalAllocationAmount += _size;
  return PlatformAllocateMem(_size);
}

void IMemFree(void* _data, u64 _size, IceMemoryTypeFlag _type)
{
  stats.typeAllocationAmounts[_type] -= _size;
  stats.totalAllocationAmount -= _size;
  PlatformFreeMem(_data);
}

void* IMemZero(void* _data, u64 _size)
{
  return PlatformZeroMem(_data, _size);
}

void* IMemCopy(void* _dst, const void* _src, u64 _size)
{
  return PlatformCopyMem(_dst, _src, _size);
}

void IMemLogStats()
{
  PlatformPrintToConsole("Total                     : %llu\n", stats.totalAllocationAmount);
  for (u32 i = 0; i < IMEM_COUNT; i++)
  {
    PlatformPrintToConsole("%-25s : %llu\n", IceMemoryTypeStrings[i], stats.typeAllocationAmounts[i]);
  }
}

