
#ifndef ICE_CORE_ECS_H_
#define ICE_CORE_ECS_H_

#include "defines.h"

#include <vector>
#include <unordered_map>

namespace Ice {

  typedef u64 Entity;

  //=========================
  // Entity
  //=========================

  static const Entity invalidEntity = 0;

  static Ice::Entity CreateEntity()
  {
    static Ice::Entity nextID = 0;
    return ++nextID;
  }

  namespace ECS {

    //=========================
    // Component
    //=========================

    template<typename Type>
    class ComponentManager
    {
    private:
      std::vector<Type> components;
      std::vector<Ice::Entity> entities;
      using indexType = u32;
      std::unordered_map<Ice::Entity, indexType> lookup;

    public:
      ComponentManager(indexType _startSize = 1)
      {
        components.reserve(_startSize);
        entities.reserve(_startSize);
        lookup.reserve(_startSize);
      }

      Type& Create(Ice::Entity _entity)
      {
        ICE_ASSERT_MSG(_entity != Ice::invalidEntity, "Invalid entity");
        ICE_ASSERT(entities.size() == components.size());
        ICE_ASSERT(lookup.size() == components.size());

        if (lookup.find(_entity) != lookup.end())
        {
          IceLogWarning("Entity %u already has a component of this type", _entity);
          return components[lookup[_entity]];
        }

        lookup[_entity] = components.size();
        components.push_back(Type());
        entities.push_back(_entity);

        return components.back();
      }

      void Remove(Ice::Entity _entity)
      {
        auto tuple = lookup.find(_entity);

        if (tuple == lookup.end())
          return;

        const indexType index = tuple->second;
        const Ice::Entity entity = entities[index];

        if (index < components.size() - 1)
        {
          //MoveItem(index, components.size() - 1);
          components[index] = std::move(components.back());
          entities[index] = entities.back();
          lookup[entities[index]] = index;
        }

        components.pop_back();
        entities.pop_back();
        lookup.erase(entity);
      }

      Type* GetComponent(Ice::Entity _entity)
      {
        auto tuple = lookup.find(_entity);

        if (tuple != lookup.end())
        {
          return &components[tuple->second];
        }

        return nullptr;
      }

      void MoveItem(indexType _fromIndex, indexType _toIndex)
      {
        ICE_ASSERT(_fromIndex < GetCount());
        ICE_ASSERT(_toIndex < GetCount());
        if (_from == _to)
          return;

        Type movedComponent = std::move(components[_fromIndex]);
        Ice::Entity movedEntity = entities[_fromIndex];

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

      bool Contains(Ice::Entity _entity) const
      {
        return lookup.find(_entity) != lookup.end();
      }

      Type& operator[](indexType _index)
      {
        return components[_index];
      }

      indexType GetCount() const
      {
        return components.size();
      }

      Ice::Entity GetEntity(indexType _index) const
      {
        return entities[_index];
      }

    };

  }
}
#endif // !ICE_CORE_ECS_H_
