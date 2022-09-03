
#ifndef ICE_CORE_ECS_ENTITY_H_
#define ICE_CORE_ECS_ENTITY_H_

#include "defines.h"

#include "core/ecs/ecs_defines.h"
#include "platform/compact_array.h"
#include "tools/flag_array.h"

#include <vector>

namespace Ice {

typedef u64 EntityComponentMask;
const u16 nullIdShort = 0xffff;
const u32 nullId = 0xffffffff;
const u64 nullIdLong = 0xffffffffffffffff;

struct Entity
{
  u32 id = Ice::nullId;
  u16 owningScene = Ice::nullIdShort;
  u16 version = Ice::nullIdShort;

  EntityComponentMask componentMask = 0;

  constexpr operator u32() const
  {
    return id;
  }

  constexpr bool operator ==(Ice::Entity& _other) const
  {
    return id == _other.id &&
           version == _other.version &&
           owningScene == _other.owningScene &&
           componentMask == _other.componentMask;
  }

  constexpr bool operator !=(Ice::Entity& _other) const
  {
    return id != _other.id ||
           version != _other.version ||
           owningScene != _other.owningScene ||
           componentMask != _other.componentMask;
  }

  template <typename T>
  T* AddComponent()
  {
    if ((componentMask & (1llu << Ice::GetComponentId<T>())) == 0)
    {
      Ice::GetComponentArray<T>().AddElementAt(id);
      componentMask |= (1llu << Ice::GetComponentId<T>());
      // Ice::Entities are technically duplicated.
      // One stored in activeEntities, the other managed by the game
      // TODO : ~!!~ See if there is a way to separate entity data (backend) and functionality (frontend)
      activeEntities[id].componentMask |= (1llu << Ice::GetComponentId<T>());
    }

    return Ice::GetComponentArray<T>().Get(id);
  }

  template <typename T>
  T* GetComponent()
  {
    return Ice::GetComponentArray<T>().Get(id);
  }

  b8 IsValid();

};

const Ice::Entity nullEntity = {Ice::nullId, Ice::nullIdShort, Ice::nullIdShort, 0};

Ice::Entity CreateEntity();

} // namespace Ice

#endif // !ICE_CORE_ECS_ENTITY_H_
