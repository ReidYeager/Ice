
#ifndef ICE_CORE_ECS_H_
#define ICE_CORE_ECS_H_

#include "defines.h"

#include <vector>
#include <unordered_map>

namespace Ice {
  namespace ECS {

    //=========================
    // Entity
    //=========================

    typedef u64 Entity;
    static const Entity invalidEntity = 0;

    static Ice::ECS::Entity CreateEntity()
    {
      static Ice::ECS::Entity nextID = 0;
      return ++nextID;
    }

    //=========================
    // Component
    //=========================

    template<typename Type>
    class ComponentManager
    {
    private:
      u32 maxCount;
      u32 currentCount = 0;
      Type* components;
      Ice::ECS::Entity* entities;

      using indexType = u16;
      std::unordered_map<Ice::ECS::Entity, indexType> lookup;

    public:
      ComponentManager()
      {
        maxCount = 0;
      }

      ComponentManager(indexType _count)
      {
        maxCount = _count;
        components = (Type*)Ice::MemoryAllocate(sizeof(Type) * maxCount);
        entities = (Ice::ECS::Entity*)Ice::MemoryAllocate(sizeof(Ice::ECS::Entity) * maxCount);
        lookup.reserve(maxCount);
      }

      void Initialize(indexType _count)
      {
        maxCount = _count;
        components = (Type*)Ice::MemoryAllocZero(sizeof(Type) * maxCount);
        entities = (Ice::ECS::Entity*)Ice::MemoryAllocZero(sizeof(Ice::ECS::Entity) * maxCount);
        lookup.reserve(maxCount);
      }

      b8 Shutdown()
      {
        Ice::MemoryFree(components);
        Ice::MemoryFree(entities);
        lookup.clear();

        return true;
      }

      Type* Create(Ice::ECS::Entity _entity)
      {
        ICE_ASSERT_MSG(_entity != Ice::ECS::invalidEntity, "Invalid entity");
        ICE_ASSERT(lookup.size() == currentCount);

        if (lookup.find(_entity) != lookup.end())
        {
          IceLogWarning("Entity %u already has a component of this type", _entity);
          return &components[lookup[_entity]];
        }

        lookup[_entity] = currentCount;
        components[currentCount] = Type();
        entities[currentCount] = _entity;
        currentCount++;

        return &components[currentCount - 1];
      }

      void Remove(Ice::ECS::Entity _entity)
      {
        auto tuple = lookup.find(_entity);

        if (tuple == lookup.end())
          return;

        const indexType index = tuple->second;
        const Ice::ECS::Entity entity = entities[index];

        if (index < currentCount - 1)
        {
          //MoveItem(index, components.size() - 1);
          components[index] = std::move(components[currentCount - 1]);
          entities[index] = entities[currentCount - 1];
          lookup[entities[index]] = index;
        }

        currentCount--;
        lookup.erase(entity);
      }

      void MoveItem(indexType _fromIndex, indexType _toIndex)
      {
        ICE_ASSERT(_fromIndex < GetCount());
        ICE_ASSERT(_toIndex < GetCount());
        if (_from == _to)
          return;

        Type movedComponent = std::move(components[_fromIndex]);
        Ice::ECS::Entity movedEntity = entities[_fromIndex];

        const int direction = _fromIndex < _toIndex ? 1 : -1;
        indexType nextIndex;
        for (indexType i = _fromIndex; i != _toIndex; i += direction)
        {
          nextIndex = i + direction;
          components[i] = std::move(components[nextIndex]);
          entities[i] = entities[nextIndex];
          lookup[entities[i]] = i;
        }

        components[_toIndex] = std::move(movedComponent);
        entities[_toIndex] = movedEntity;
        lookup[movedEntity] = _toIndex;
      }

      Type* GetComponent(Ice::ECS::Entity _entity)
      {
        if (!_entity)
          return nullptr;

        auto tuple = lookup.find(_entity);

        if (tuple != lookup.end())
        {
          return &components[tuple->second];
        }

        return nullptr;
      }

      indexType GetIndex(Ice::ECS::Entity _entity)
      {
        return lookup.find(_entity)->second;
      }

      bool Contains(Ice::ECS::Entity _entity) const
      {
        return lookup.find(_entity) != lookup.end();
      }

      Type& operator[](indexType _index)
      {
        return components[_index];
      }

      indexType GetCount() const
      {
        return currentCount;
      }

      Ice::ECS::Entity GetEntity(indexType _index) const
      {
        return entities[_index] * (currentCount > _index);
      }

      const Ice::ECS::Entity* GetEntityArray()
      {
        return entities;
      }

    };

  }
}
#endif // !ICE_CORE_ECS_H_
