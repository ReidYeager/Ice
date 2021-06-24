
#include "core/memory_manager.h"
#include "platform/platform.h"
#include "logger.h"

// TEMPORARY -- Replace with a proper logging system
#include <stdio.h>

IceMemoryManager MemoryManager;

void IceMemoryManager::Initialize()
{
  Zero(MemoryManager.GetStats(), sizeof(MemoryStatistics));\
  IcePrint("Initialized MemoryManagement system");
}

void IceMemoryManager::Shutdown()
{
  PrintStats();
  IcePrint("Shutdown MemoryManagement system");
}

IceMemoryManager::MemoryStatistics* IceMemoryManager::GetStats()
{
  return &stats;
}

void IceMemoryManager::PrintStats()
{
  const char* categoryNames[IMEM_TYPE_COUNT] = {
    "MEM_TYPE_UNKNOWN", "MEM_TYPE_BUFFER"
  };

  MemoryStatistics* stats = MemoryManager.GetStats();

  for (u32 i = 0; i < IMEM_TYPE_COUNT; i++)
  {
    printf("%-25s : %llu %s\n", categoryNames[i], stats->categoryAllocations[i], "B");
  }
  printf("%-25s : %llu %s\n", "Total", stats->totalMemoryAllocated, "B");
}

void* IceMemoryManager::Allocate(u32 size, IceMemoryTypeFlag flag)
{
  MemoryStatistics* stats = MemoryManager.GetStats();
  stats->totalMemoryAllocated += size;
  stats->categoryAllocations[flag] += size;
  return Platform.AllocateMem(size);
}

void IceMemoryManager::Free(void* data, u32 size, IceMemoryTypeFlag flag)
{
  MemoryStatistics* stats = MemoryManager.GetStats();
  stats->totalMemoryAllocated -= size;
  stats->categoryAllocations[flag] -= size;
  Platform.FreeMem(data);
}

void IceMemoryManager::Zero(void* data, u32 size)
{
  Platform.ZeroMem(data, size);
}

void IceMemoryManager::Copy(void* dst, void* src, u32 size)
{
  Platform.CopyMem(dst, src, size);
}
