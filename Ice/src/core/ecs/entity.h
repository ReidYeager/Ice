
#ifndef ICE_CORE_ECS_ENTITY_H_
#define ICE_CORE_ECS_ENTITY_H_

#include "defines.h"

#include "core/ecs/ecs_defines.h"
#include "core/platform/compact_array.h"
#include "tools/flag_array.h"
//#include "math/transform.h"

#include <vector>

namespace Ice {

typedef u64 EntityComponentMask;

struct Entity
{
  u32 id = Ice::null32;
  u16 owningScene = Ice::null16;
  u16 version = Ice::null16;

  EntityComponentMask componentMask = 0;

  constexpr operator u32() const
  {
    return id;
  }

  constexpr bool operator ==(Ice::Entity& _other) const
  {
    return id == _other.id
      && version == _other.version
      && owningScene == _other.owningScene
      && componentMask == _other.componentMask;
  }

  constexpr bool operator !=(Ice::Entity& _other) const
  {
    return id != _other.id
      || version != _other.version
      || owningScene != _other.owningScene
      || componentMask != _other.componentMask;
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

const Ice::Entity nullEntity = { Ice::null32, Ice::null16, Ice::null16, 0 };

Ice::Entity CreateEntity();
Ice::Entity GetEntity(u32 _id);

} // namespace Ice

#endif // !ICE_CORE_ECS_ENTITY_H_
