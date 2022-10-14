
#ifndef ICE_TOOLS_POOL_H_
#define ICE_TOOLS_POOL_H_

#include "defines.h"
#include "core/platform/platform.h"

namespace Ice {

template<typename T>
class CompactPool
{
  private:
  T* data = nullptr;
  u32* indexMap = nullptr;
  Ice::FlagArray indexAvailability;

  u32 allocatedElementCount = 0;
  u32 usedElementCount = 0;
  u32 indexCount = 0;

  // When false, prevents automatic resizing but not manual
  b8 canGrow = false;

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
  CompactPool(u32 _count = 1, b8 _canGrow = false)
  {
    data = (T*)Ice::MemoryAllocZero(_count * sizeof(T));
    indexMap = (u32*)Ice::MemoryAllocate(_count * sizeof(u32));
    indexAvailability.Resize(_count, true);
    allocatedElementCount = _count;
    indexCount = _count;
    canGrow = _canGrow;
  }

  CompactPool(T* _data, u32 _count, b8 _canGrow = false)
  {
    data = (T*)Ice::MemoryAllocZero(_count * sizeof(T));
    Ice::MemoryCopy(_data, data, _count);
    indexMap = (u32*)Ice::MemoryAllocate(_count * sizeof(u32));
    indexAvailability.Resize(_count, true);
    allocatedElementCount = _count;
    indexCount = _count;
    canGrow = _canGrow;
  }

  ~CompactPool()
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

  void SetCanGrow(b8 _value)
  {
    canGrow = _value;
  }

  T& GetNewElement(u32* _elementIndex = nullptr)
  {
    if (usedElementCount >= allocatedElementCount)
    {
      if (!canGrow)
      {
        ICE_ABORT("Pool reached limit");
      }

      ResizeData(allocatedElementCount * 2);
      ResizeMap(allocatedElementCount);
    }

    u32 index = indexAvailability.FirstIndexWithValue(1);
    u32 dataIndex = usedElementCount;

    indexAvailability.Set(index, 0);
    indexMap[index] = dataIndex;
    data[dataIndex] = T();
    usedElementCount++;

    if (_elementIndex != nullptr)
      *_elementIndex = index;

    return data[dataIndex];
  }

  void ReturnElement(u32 _index)
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

  constexpr b8 IsEmpty() const
  {
    return usedElementCount == allocatedElementCount;
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

}

#endif // !ICE_TOOLS_POOL_H_
