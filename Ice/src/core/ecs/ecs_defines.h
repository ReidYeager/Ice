
#ifndef ICE_CORE_ECS_ECS_DEFINES_H_
#define ICE_CORE_ECS_ECS_DEFINES_H_

#include "defines.h"
#include "platform/compact_array.h"

namespace Ice {

struct EntityBackend
{
  u32 id;
  u16 owningScene;
  u16 version;

  EntityComponentMask componentMask;
};

extern Ice::CompactArray<Ice::EntityBackend> activeEntities;
extern std::vector<Ice::EntityBackend> availableEntities;

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
