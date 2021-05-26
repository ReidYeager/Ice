
#ifndef CORE_MEMORY_MANAGER_H
#define CORE_MEMORY_MANAGER_H 1

#include "defines.h"

enum IceMemoryTypeBits
{
  IMEM_UNKNOWN,
  IMEM_ARRAY,
  IMEM_TEXTURE,
  IMEM_BUFFER,
  IMEM_COUNT
};
typedef IceFlag IceMemoryTypeFlag;

// TODO : ? Include a vector of void* to all allocated memory ?

// Initializes memory statistics
void IMemInitialize();
// 
void IMemShutdown();
// Allocates a block of memory on the stack of size _size
void* IMemAllocate(u64 _size, IceMemoryTypeFlag _type);
// Frees a block of memory on the stack
void  IMemFree(void* _data, u64 _size, IceMemoryTypeFlag _type);
// Sets a block of memory of size _size to all 0
void* IMemZero(void* _data, u64 _size);
// Copies _size bytes from _src to _dst
void* IMemCopy(void* _dst, const void* _src, u64 _size);
// Prints the current data usage in each memory type
void IMemLogStats();

#endif // !CORE_MEMORY_MANAGER_H
