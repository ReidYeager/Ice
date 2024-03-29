
#ifndef ICE_PLATFORM_COMPACT_ARRAY_H_
#define ICE_PLATFORM_COMPACT_ARRAY_H_

#include "defines.h"

#include "core/platform/platform.h"
#include "tools/flag_array.h"

#include <bitset>
#include <vector>

namespace Ice {

template<typename T>
class CompactArray
{
  private:
  T* data = nullptr;
  u32* indexMap = nullptr;
  Ice::FlagArray indexAvailability;

  u32 allocatedElementCount = 0;
  u32 usedElementCount = 0;
  u32 indexCount = 0;

  constexpr u32 BackIndex() const
  {
    return usedElementCount - 1;
  }

  void ResizeData(u32 _newCount)
  {
    T* old = data;
    data = (T*)Ice::MemoryAllocZero(_newCount * sizeof(T));

    usedElementCount = min(usedElementCount, _newCount);
    Ice::MemoryCopy(old, data, usedElementCount * sizeof(T));
    Ice::MemoryFree(old);

    allocatedElementCount = _newCount;
  }

  void ResizeMap(u32 _newCount)
  {
    u32* oldMap = indexMap;
    indexMap = (u32*)Ice::MemoryAllocate(_newCount * sizeof(u32));
    indexAvailability.Resize(_newCount, true);

    Ice::MemoryCopy(oldMap, indexMap, indexCount * sizeof(u32));
    indexCount = _newCount;
  }

public:
  CompactArray(u32 _count)
  {
    data = (T*)Ice::MemoryAllocZero(_count * sizeof(T));
    indexMap = (u32*)Ice::MemoryAllocate(_count * sizeof(u32));
    indexAvailability.Resize(_count, true);
    allocatedElementCount = _count;
    indexCount = _count;
  }

  CompactArray(const Ice::CompactArray<T>& _other)
  {
    u32 _count;
    T* _data = _other.GetArray(_count);

    data = (T*)Ice::MemoryAllocZero(_count * sizeof(T));
    Ice::MemoryCopy(_data, data, _count);
    indexMap = (u32*)Ice::MemoryAllocate(_count * sizeof(u32));
    indexAvailability.Resize(_count, true);
    allocatedElementCount = _count;
    indexCount = _count;
  }

  ~CompactArray()
  {
    if (allocatedElementCount)
      Shutdown();
  }

  void Shutdown()
  {
    Ice::MemoryFree(data);
    Ice::MemoryFree(indexMap);
    allocatedElementCount = 0;
    usedElementCount = 0;
    indexCount = 0;
    indexAvailability.Shutdown();
  }

  u32 AddElementAt(u32 _index, T _initValue = {})
  {
    if (_index >= indexCount)
    {
      u32 newCount = indexCount;
      while (newCount <= _index)
      {
        newCount *= 2;
      }

      ResizeMap(newCount);
    }
    if (usedElementCount >= allocatedElementCount)
    {
      ResizeData(allocatedElementCount * 2);
    }

    if (!indexAvailability.Get(_index))
    {
      IceLogWarning("Index %u unavailable in compact array", _index);
      DebugBreak();
      return -1;
    }

    u32 dataIndex = usedElementCount;

    indexAvailability.Set(_index, 0);
    indexMap[_index] = dataIndex;
    data[dataIndex] = _initValue;
    usedElementCount++;

    return _index;
  }

  u32 AddElement(T _initValue = {})
  {
    if (usedElementCount >= allocatedElementCount)
    {
      ResizeData(allocatedElementCount * 2);
    }
    if (indexCount < allocatedElementCount)
    {
      ResizeMap(allocatedElementCount);
    }

    u32 index = indexAvailability.FirstIndexWithValue(1);
    u32 dataIndex = usedElementCount;

    indexAvailability.Set(index, 0);
    indexMap[index] = dataIndex;
    data[dataIndex] = _initValue;
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

  u32 GetMappedIndex(u32 _index)
  {
    assert(_index < indexCount);
    return indexMap[_index];
  }

  T& operator [](u32 _index)
  {
    assert(_index < indexCount);
    return data[indexMap[_index]];
  }

  T* Get(u32 _index)
  {
    assert(_index < indexCount);
    return &data[indexMap[_index]];
  }

  T* GetArray(u32* _count = nullptr)
  {
    if (_count != nullptr)
      *_count = usedElementCount;

    return data;
  }

  constexpr u32 Size() const
  {
    return usedElementCount;
  }

  constexpr u32 GetAllocatedSize() const
  {
    return allocatedElementCount;
  }

  void Resize(u32 _newCount)
  {
    ResizeData(_newCount);
    ResizeMap(_newCount);
  }

  //=========================
  // Iterator
  //=========================

  struct Iterator
  {
    u32 index;
    u32 maxCount;
    T* data;

    Iterator(u32 _index, u32 _maxCount, T* _data)
    {
      index = _index;
      maxCount = _maxCount;
      data = _data;
    }

    T& operator *() const
    {
      return data[index];
    }

    Iterator& operator ++()
    {
      if (index < maxCount)
        index++;

      return *this;
    }

    bool operator !=(const Iterator& _other)
    {
      return index != _other.index;
    }

    bool operator ==(const Iterator& _other)
    {
      return index == _other.index;
    }
  };

  const Iterator begin() const
  {
    return Iterator(0, usedElementCount, data);
  }

  const Iterator end() const
  {
    return Iterator(usedElementCount, usedElementCount, data);
  }

};

} // namespace Ice

#endif // !ICE_PLATFORM_COMPACT_ARRAY_H_
