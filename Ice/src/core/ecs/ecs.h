
#ifndef ICE_CORE_ECS_ECS_H_
#define ICE_CORE_ECS_ECS_H_

#include "core/ecs/entity.h"
#include "tools/compact_array.h"

#include <vector>

namespace Ice {

template <typename... types>
class SceneView
{
public:
  Ice::EntityComponentMask mask = 0;

  SceneView()
  {
    u32 ids[] = { Ice::GetComponentId<types>() ... };

    for (u32 i = 0; i < (sizeof...(types)); i++)
    {
      mask |= (1llu << ids[i]);
    }
  }

  struct Iterator
  {
    Ice::EntityComponentMask mask;
    u32 entityIndex = 0;

    Iterator(Ice::EntityComponentMask _mask, u32 _index)
    {
      mask = _mask;
      entityIndex = _index;
    }

    Ice::Entity& operator *() const
    {
      return activeEntities[entityIndex];
    }

    Iterator& operator ++()
    {
      do
      {
        entityIndex++;
      } while (entityIndex < activeEntities.Size() &&
               (activeEntities[entityIndex].id == nullEntity.id ||
                (activeEntities[entityIndex].componentMask & mask) != mask));

      return *this;
    }

    bool operator !=(const Iterator& _other) const
    {
      return entityIndex != _other.entityIndex && entityIndex != activeEntities.Size();
    }

    bool operator ==(const Iterator& _other) const
    {
      return entityIndex == _other.entityIndex || entityIndex == activeEntities.Size();
    }

  };

  b8 isValidIndex(u32 _index) const
  {
    return activeEntities[_index].id != nullEntity.id &&
      (activeEntities[_index].componentMask & mask) == mask;
  }

  const Iterator begin() const
  {
    u32 startIndex = 0;

    // Skip to first valid entity
    while (startIndex < activeEntities.Size() &&
           (activeEntities[startIndex].id == nullEntity.id ||
            (activeEntities[startIndex].componentMask & mask) != mask))
    {
      startIndex++;
    }

    return Iterator(mask, startIndex);
  }

  const Iterator end() const
  {
    return Iterator(mask, (u32)activeEntities.Size());
  }

};

} // namespace Ice

#endif // !ICE_CORE_ECS_ECS_H_
