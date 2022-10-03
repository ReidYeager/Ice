
#ifndef ICE_CORE_ECS_ECS_DEFINES_H_
#define ICE_CORE_ECS_ECS_DEFINES_H_

#include "defines.h"
#include "tools/compact_array.h"

namespace Ice {

struct Entity;

extern Ice::CompactArray<Ice::Entity> activeEntities;
extern std::vector<Ice::Entity> availableEntities;

extern u32 componentCount;
template<typename T>
u32 GetComponentId()
{
  static u32 thisId = Ice::componentCount++;
  return thisId;
}

template<typename T>
Ice::CompactArray<T>& GetComponentArray()
{
  static Ice::CompactArray<T> thisCompact(2);
  return thisCompact;
}

} // namespace Ice

#endif // !define ICE_CORE_ECS_ECS_DEFINES_H_
