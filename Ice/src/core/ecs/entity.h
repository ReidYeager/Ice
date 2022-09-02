
#ifndef ICE_CORE_ECS_ENTITY_H_
#define ICE_CORE_ECS_ENTITY_H_

#include "defines.h"

#include "core/ecs/ecs_defines.h"
#include "platform/compact_array.h"
#include "tools/flag_array.h"

#include <vector>

namespace Ice {

typedef u64 EntityComponentMask;

struct Entity
{
  u32 id;
  u16 owningScene;
  u16 version;

  EntityComponentMask componentMask;

  constexpr operator u32() const
  {
    return id;
  }

  constexpr bool operator ==(Ice::Entity& _other) const
  {
    return id == _other.id && version == _other.version && owningScene == _other.owningScene;
  }

  constexpr bool operator !=(Ice::Entity& _other) const
  {
    return id != _other.id || version != _other.version || owningScene != _other.owningScene;
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

};

const Ice::Entity nullEntity = {0xffffffff, 0xffff, 0xffff};

Ice::Entity CreateEntity();

} // namespace Ice

#endif // !ICE_CORE_ECS_ENTITY_H_
