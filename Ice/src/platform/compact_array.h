
#ifndef ICE_PLATFORM_COMPACT_ARRAY_H_
#define ICE_PLATFORM_COMPACT_ARRAY_H_

#include "defines.h"

#include "platform/platform.h"
#include "tools/flag_array.h"

#include <bitset>
#include <vector>

namespace Ice {

// NOTE : Very unstable. Elements of the array can change at any time.
// TODO : Need to find a way to accomplish this without breaking connections to its elements

template<class T>
class CompactArray
{
  public:
  T* data = nullptr;
  u32* indexMap = nullptr;
  Ice::FlagArray indexAvailability;

  u32 allocatedElementCount = 0;
  u32 usedElementCount = 0;

  constexpr u32 BackIndex()
  {
    return usedElementCount - 1;
  }

public:
  CompactArray(u32 _count)
  {
    allocatedElementCount = _count;
    data = (T*)Ice::MemoryAllocate(allocatedElementCount * sizeof(T));
    indexMap = (u32*)Ice::MemoryAllocZero(allocatedElementCount * sizeof(u32));
    indexAvailability.Resize(allocatedElementCount, true);
  }

  u32 AddElement(T _newElement, u32 _index = -1)
  {
    u32 index = -1;

    if (_index == -1)
    {
      index = indexAvailability.FirstIndexWithValue(1);
    }
    else
    {
      if (!indexAvailability.Get(_index))
      {
        IceLogWarning("Index %u unavailable in compact array", _index);
        DebugBreak();
        return -1;
      }

      index = _index;
    }

    u32 dataIndex = usedElementCount;

    indexAvailability.Set(index, 0);
    indexMap[index] = dataIndex;
    data[dataIndex] = _newElement;
    usedElementCount++;

    return index;
  }

  u32 AddElement(u32 _index = -1)
  {
    u32 index = indexAvailability.FirstIndexWithValue(1);
    u32 dataIndex = usedElementCount;

    indexAvailability.Set(index, 0);
    indexMap[index] = dataIndex;
    data[dataIndex] = {};
    usedElementCount++;

    return index;
  }

  void RemoveAt(u32 _index)
  {
    assert(_index < usedElementCount);

    u32 dataIndex = indexMap[_index];
    u32 dataBackIndex = usedElementCount - 1;

    // Swap removed & back in data
    data[dataIndex] = data[dataBackIndex];

    // Find back index in map
    u32 mapBackIndex = 0;
    while (mapBackIndex < usedElementCount && indexMap[mapBackIndex] != dataBackIndex)
    {
      mapBackIndex++;
    }
    indexMap[_index] = indexMap[mapBackIndex];

    indexAvailability.Set(mapBackIndex, 1);
    usedElementCount--;
  }

  T& operator [](u32 _index)
  {
    assert(_index < allocatedElementCount);
    return data[indexMap[_index]];
  }

  T* Get(u32 _index)
  {
    assert(_index < allocatedElementCount);
    return &data[indexMap[_index]];
  }

  T* GetArray(u32* _count = nullptr)
  {
    if (_count != nullptr)
      *_count = usedElementCount;

    return data;
  }

  u32 GetAllocatedSize()
  {
    return allocatedElementCount;
  }

  void Resize(u64 _newSize)
  {
    data = (T*)realloc(sizeof(T) * _newSize);
    allocatedElementCount = _newSize;
  }

};

} // namespace Ice

#endif // !ICE_PLATFORM_COMPACT_ARRAY_H_
