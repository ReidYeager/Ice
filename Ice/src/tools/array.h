
#ifndef ICE_TOOLS_ARRAY_H_
#define ICE_TOOLS_ARRAY_H_

#include "defines.h"
#include "platform/platform.h"

namespace Ice {

template<typename T>
class Array
{
private:
  T* data = nullptr;

  u32 allocatedElementCount = 0;
  u32 usedElementCount = 0;

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

public:
  Array(u32 _count = 1)
  {
    data = (T*)Ice::MemoryAllocZero(_count * sizeof(T));
    allocatedElementCount = _count;
  }

  Array(T* _data, u32 _count)
  {
    data = (T*)Ice::MemoryAllocZero(_count * sizeof(T));
    allocatedElementCount = _count;

    Ice::MemoryCopy(_data, data, _count * sizeof(T));
  }

  ~Array()
  {
    if (allocatedElementCount)
      Shutdown();
  }

  void Shutdown()
  {
    Ice::MemoryFree(data);
    allocatedElementCount = 0;
    usedElementCount = 0;
  }

  T& PushBack(T _value)
  {
    if (usedElementCount >= allocatedElementCount)
    {
      ResizeData(allocatedElementCount * 2);
    }

    data[usedElementCount] = _value;
    usedElementCount++;

    return data[usedElementCount - 1];
  }

  T& Back()
  {
    return data[usedElementCount - 1];
  }

  T& operator [](u32 _index)
  {
    assert(_index < usedElementCount);
    return data[_index];
  }

  T* Get(u32 _index)
  {
    assert(_index < usedElementCount);
    return &data[_index];
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

#endif // !ICE_TOOLS_ARRAY_H_
