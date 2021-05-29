
#include "core/memory_manager.h"
#include "platform/platform.h"

// TEMPORARY -- Replace with a proper logging system
#include <stdio.h>

MemoryManager* MemoryManager::instance;

void MemoryManager::Initialize()
{
  Zero(GetInstance()->GetStats(), sizeof(MemoryStatistics));
}

void MemoryManager::Shutdown()
{
  PrintStats();
}

MemoryManager* MemoryManager::GetInstance()
{
  if (!instance)
  {
    instance = new MemoryManager();
  }

  return instance;
}

MemoryManager::MemoryStatistics* MemoryManager::GetStats()
{
  return &stats;
}

void MemoryManager::PrintStats()
{
  const char* categoryNames[IMEM_TYPE_COUNT] = {
    "MEM_TYPE_UNKNOWN", "MEM_TYPE_BUFFER"
  };

  MemoryManager* mm = MemoryManager::GetInstance();
  MemoryStatistics* stats = mm->GetStats();

  for (u32 i = 0; i < IMEM_TYPE_COUNT; i++)
  {
    printf("%-25s : %llu %s\n", categoryNames[i], stats->categoryAllocations[i], "B");
  }
  printf("%-25s : %llu %s\n", "Total", stats->totalMemoryAllocated, "B");
}

void* MemoryManager::Allocate(u32 size, IceMemoryTypeFlag flag)
{
  MemoryStatistics* stats = GetInstance()->GetStats();
  stats->totalMemoryAllocated += size;
  stats->categoryAllocations[flag] += size;
  return Platform::AllocateMem(size);
}

void MemoryManager::Free(void* data, u32 size, IceMemoryTypeFlag flag)
{
  MemoryStatistics* stats = GetInstance()->GetStats();
  stats->totalMemoryAllocated -= size;
  stats->categoryAllocations[flag] -= size;
  Platform::FreeMem(data);
}

void MemoryManager::Zero(void* data, u32 size)
{
  Platform::ZeroMem(data, size);
}
