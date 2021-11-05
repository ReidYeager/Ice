
#include "logger.h"

#include "core/memory_manager.h"
#include "platform/platform.h"

IceMemoryManager MemoryManager;

void IceMemoryManager::Initialize()
{
  Zero(MemoryManager.GetStats(), sizeof(MemoryStatistics));\
  IceLogInfo("Initialized MemoryManagement system");
}

void IceMemoryManager::Shutdown()
{
  PrintStats();
  IceLogInfo("Shutdown MemoryManagement system");
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
    IceLogInfo("%-25s : %llu %s", categoryNames[i], stats->categoryAllocations[i], "B");
  }
  IceLogInfo("%-25s : %llu %s", "Total", stats->totalMemoryAllocated, "B");
}

void* IceMemoryManager::Allocate(u32 size, IceMemoryTypeFlag flag)
{
  MemoryStatistics* stats = MemoryManager.GetStats();
  stats->totalMemoryAllocated += size;
  stats->categoryAllocations[flag] += size;
  return IcePlatform::AllocateMem(size);
}

void IceMemoryManager::Free(void* data, u32 size, IceMemoryTypeFlag flag)
{
  MemoryStatistics* stats = MemoryManager.GetStats();
  stats->totalMemoryAllocated -= size;
  stats->categoryAllocations[flag] -= size;
  IcePlatform::FreeMem(data);
}

void IceMemoryManager::Zero(void* data, u32 size)
{
  IcePlatform::ZeroMem(data, size);
}

void IceMemoryManager::Copy(void* dst, void* src, u32 size)
{
  IcePlatform::CopyMem(dst, src, size);
}
