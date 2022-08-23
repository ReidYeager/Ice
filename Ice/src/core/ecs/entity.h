
#ifndef ICE_CORE_ECS_ENTITY_H_
#define ICE_CORE_ECS_ENTITY_H_

#include "defines.h"

#include "platform/compact_array.h"

#include <vector>

namespace Ice {

struct Entity
{
  u32 id;
  u16 owningScene;
  u16 version;

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
};

// TODO : ~!!~ Let the game define max entity count || Let entity array expand
const u32 maxEntities = 2048;
const Ice::Entity nullEntity = {0xffffffff, 0xffff, 0xffff};

typedef u64 componentMask;
extern u32 establishedComponentCount;

template <typename T>
u32 GetComponentId()
{
  static u32 componentId = Ice::establishedComponentCount++;
  return componentId;
}

template <typename T>
Ice::CompactArray<T>& GetComponentArray()
{
  static Ice::CompactArray<T> arr(maxEntities);
  return arr;
}

struct EntityDescription
{
  Ice::Entity entity;
  componentMask componentMask;
};

class Scene
{
public:
  std::vector<Ice::EntityDescription> activeEntities;
  std::vector<u32> availableEntities;
  //std::vector<Ice::MemoryPool*> componentPools;

public:

  Scene()
  {
    activeEntities.reserve(maxEntities);
  }

  // Entities =====

  Ice::Entity CreateEntity()
  {
    // Check for destroyed entities to use first
    if (!availableEntities.empty())
    {
      u32 av = availableEntities.back();
      availableEntities.pop_back();

      activeEntities[av].entity.id = av;
      return activeEntities[av].entity;
    }

    // Destroyed entities still count against total entity count
    // Can only hit limit if available is empty
    if (activeEntities.size() >= maxEntities)
    {
      IceLogError("Max entity count reached : %u", maxEntities);
      return nullEntity;
    }

    // Create new entity
    activeEntities.push_back({ { (u32)activeEntities.size(), 0 }, 0});
    return activeEntities.back().entity;
  }

  void DestroyEntity(Ice::Entity _entity)
  {
    if (_entity != activeEntities[_entity].entity)
      return;

    // Clear old entity
    activeEntities[_entity].entity.id = nullEntity.id;
    activeEntities[_entity].entity.version++;

    // Add entity to available
    availableEntities.push_back(_entity);
  }

  // Components =====

  //template <typename T>
  //Ice::GetComponentArray<T>()* GetComponentArray()
  //{
  //  return &Ice::GetComponentArray<T>();
  //}

  template <typename T>
  T* GetComponent(Ice::Entity _entity)
  {
    // Check if entity is valid
    if (_entity != activeEntities[_entity].entity)
    {
      IceLogInfo("Entity is invalid in 'Get component'");
      return nullptr;
    }

    u32 componentId = GetComponentId<T>();

    if (activeEntities[_entity].componentMask & (1llu << componentId))
    {
      return &Ice::GetComponentArray<T>()[_entity.id];
    }
    return nullptr;
  }

  template <typename T>
  T* AddComponent(Ice::Entity _entity)
  {
    // Check if entity is valid
    if (_entity != activeEntities[_entity].entity)
    {
      IceLogInfo("Entity is invalid in 'Add component'");
      return nullptr;
    }

    u32 componentId = GetComponentId<T>();
    Ice::CompactArray<T>& v = Ice::GetComponentArray<T>();
    v.AddElementAt(_entity.id);

    // Add the component to the entity
    activeEntities[_entity].componentMask |= (1llu << componentId);

    // Initialize the component data
    return &v[_entity.id];
  }

  template <typename T>
  void RemoveComponent(Ice::Entity _entity)
  {
    // Check if entity is valid
    if (_entity != activeEntities[_entity].entity)
    {
      IceLogInfo("Entity is invalid in 'Remove component'");
      return nullptr;
    }

    // Remove the component from the entity
    activeEntities[_entity].componentMask &= ~(1 << GetComponentId<T>());
  }
};

template <typename... types>
class SceneView
{
public:
  Ice::Scene* scene = nullptr;
  componentMask mask = 0;

  SceneView(Ice::Scene& _scene) : scene(&_scene)
  {
    u32 ids[] = { GetComponentId<types>() ... };

    for (u32 i = 0; i < (sizeof...(types)); i++)
    {
      mask |= (1llu << ids[i]);
    }
  }

  struct Iterator
  {
    Ice::Scene* scene;
    componentMask mask;
    u32 entityIndex = 0;

    Iterator(Ice::Scene* _scene, componentMask _mask, u32 _index)
    {
      scene = _scene;
      mask = _mask;
      entityIndex = _index;
    }

    Ice::Entity& operator *() const
    {
      return scene->activeEntities[entityIndex].entity;
    }

    Iterator& operator ++()
    {
      do
      {
        entityIndex++;
      } while (entityIndex < scene->activeEntities.size() &&
               (scene->activeEntities[entityIndex].entity.id == nullEntity.id ||
                (scene->activeEntities[entityIndex].componentMask & mask) != mask));

      return *this;
    }

    bool operator !=(const Iterator& _other) const
    {
      return entityIndex != _other.entityIndex && entityIndex != scene->activeEntities.size();
    }

    bool operator ==(const Iterator& _other) const
    {
      return entityIndex == _other.entityIndex || entityIndex == scene->activeEntities.size();
    }

  };

  b8 isValidIndex(u32 _index) const
  {
    return scene->activeEntities[_index].entity.id != nullEntity.id &&
      (scene->activeEntities[_index].componentMask & mask) == mask;
  }

  const Iterator begin() const
  {
    u32 startIndex = 0;

    // Skip to first valid entity
    while (startIndex < scene->activeEntities.size() &&
           (scene->activeEntities[startIndex].entity.id == nullEntity.id ||
            (scene->activeEntities[startIndex].componentMask & mask) != mask))
    {
      startIndex++;
    }

     return Iterator(scene, mask, startIndex);
  }

  const Iterator end() const
  {
    return Iterator(scene, mask, (u32)scene->activeEntities.size());
  }

};

} // namespace Ice

#endif // !ICE_CORE_ECS_ENTITY_H_
