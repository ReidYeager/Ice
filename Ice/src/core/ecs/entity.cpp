
#include "defines.h"

#include "core/ecs/entity.h"

#include "tools/logger.h"

#include <vector>

u32 Ice::componentCount = 0;
Ice::CompactArray<Ice::Entity> Ice::activeEntities(2);
std::vector<Ice::Entity> Ice::availableEntities(0);

Ice::Entity Ice::CreateEntity()
{
  // Check for destroyed entities to use first
  if (availableEntities.size() != 0)
  {
    Ice::Entity av = availableEntities.back();
    availableEntities.pop_back();

    av.version++;
    activeEntities.AddElement(av);
    return activeEntities[av];
  }

  // Destroyed entities still count against total entity count
  // Can only hit limit if available is empty
  //if (activeEntities.size() >= maxEntities)
  //{
  //  IceLogError("Max entity count reached : %u", maxEntities);
  //  return nullEntity;
  //}

  // Create new entity
  Ice::Entity e{ activeEntities.size(), 0, 0, 0 };

  u32 index = activeEntities.AddElement(e);
  return activeEntities[index];
}

b8 Ice::Entity::IsValid()
{
  // TODO : ? Entity::IsValid() can not be constexpr. Should only check against nullEntity?
  return (*this != Ice::nullEntity) && (*this == Ice::activeEntities[id]);
}
