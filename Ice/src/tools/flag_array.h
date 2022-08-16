
#ifndef ICE_TOOLS_FLAG_ARRAY_H_
#define ICE_TOOLS_FLAG_ARRAY_H_

#include "defines.h"

#include "platform/platform.h"

namespace Ice {

class FlagArray
{
private:
  char* data = nullptr;
  u32 byteCount = 0;
  u32 flagCount = 0;

  constexpr u32 ValueToByte(u32 _value) { return _value / 8; }
  constexpr u32 ValueToBit(u32 _value)
  {
    return _value % 8;
  }

public:
  FlagArray(u32 _flagCount, b8 _initialValue = false)
  {
    Resize(_flagCount, _initialValue);
  }

  FlagArray() { }

  ~FlagArray()
  {
    flagCount = 0;
    byteCount = 0;
    Ice::MemoryFree(data);
  }

  void Resize(u32 _flagCount, b8 _initialValue = false)
  {
    if (data != nullptr)
      Ice::MemoryFree(data);

    flagCount = _flagCount;
    byteCount = (flagCount + 7) / 8; // Bits to bytes, rounding up

    data = (char*)Ice::MemoryAllocate(byteCount);
    Ice::MemorySet(data, byteCount, (~0) * _initialValue);
  }

  b8 Get(u32 _index)
  {
    assert(_index < flagCount);
    return (data[ValueToByte(_index)] >> ValueToBit(_index)) & 1;
  }

  void Set(u32 _index, b8 _value)
  {
    assert(_index < flagCount);
    _value = _value != 0; // Ensure the value is only 1 or 0

    b8 changeNeeded = ((data[ValueToByte(_index)] >> ValueToBit(_index)) & 1) != _value;

    data[ValueToByte(_index)] ^= (changeNeeded << ValueToBit(_index));
  }

  b8 Toggle(u32 _index)
  {
    assert(_index < flagCount);
    data[ValueToByte(_index)] ^= (1 << ValueToBit(_index));
    return Get(_index);
  }

  u32 FirstIndexWithValue(b8 _value)
  {
    _value = _value != 0;

    u32 byteIndex = 0;

    // Either a full (0xFF) or empty (0x00) byte depending on the value being searched for
    const char ignoredByte = 0xFF * !_value;

    while (data[byteIndex] == ignoredByte && byteIndex < byteCount)
    {
      byteIndex++;
    }

    if (byteIndex >= byteCount)
    {
      IceLogWarning("No flags with desired value of %u", _value);
      return -1;
    }

    const char byteValue = data[byteIndex];
    u32 bitIndex = 0;
    u32 maxBitIndex = ValueToBit(flagCount);

    // The current bit is full so check all bits
    if (maxBitIndex == 0)
      maxBitIndex = 8;

    while (((byteValue >> bitIndex) & 1) != _value && bitIndex < maxBitIndex)
    {
      bitIndex++;
    }

    if (bitIndex >= maxBitIndex)
    {
      IceLogWarning("No flags with desired value of %u", _value);
      return -1;
    }

    return bitIndex + (8 * byteIndex);
  }

  // Set all bits to the input value
  void Clear(b8 _value = false)
  {
    _value = _value != 0;

    Ice::MemorySet(data, byteCount, (~0) * _value);
  }

};

} // namespace Ice

#endif // !ICE_TOOLS_FLAG_ARRAY_H_
