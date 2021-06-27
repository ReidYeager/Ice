
#include "logger.h"
#include "core/event.h"
#include "core/memory_manager.h"

IceEventManager EventManager;

void IceEventManager::Initialize()
{
  //MemoryManager.Zero(state, sizeof(CodeEvents) * Ice_Event_Max);
  IcePrint("Initialized Event system");
}

void IceEventManager::Shutdown()
{
  for (u16 i = 0; i < Ice_Event_Max; i++)
  {
    if (state[i].registeredEvents.size() != 0)
    {
      state[i].registeredEvents.clear();
    }
  }
  IcePrint("Shutdown Event system");
}

bool IceEventManager::Register(u16 _eCode, void* _listener, EventCallback _callaback)
{
  // Ensure no duplicate
  for (const auto& e : state[_eCode].registeredEvents)
  {
    if (e.listener == _listener && e.callback == _callaback)
    {
      // Warn of duplicate
      return false;
    }
  }

  RegisteredEvent re  = {_listener, _callaback};
  state[_eCode].registeredEvents.push_back(re);

  return true;
}

bool IceEventManager::Unregister(u16 _eCode, void* _listener, EventCallback _callaback)
{
  u32 size = state[_eCode].registeredEvents.size();
  for (u32 i = 0; i < size; i++)
  {
    std::vector<RegisteredEvent>& e = state[_eCode].registeredEvents;
    if (e[i].listener == _listener && e[i].callback == _callaback)
    {
      RegisteredEvent back = e.back();
      e.pop_back();

      // If i was not the final event, replace it
      if (i < (size - 1))
      {
        e[i] = back;
      }
      return true;
    }
  }

  // Failed to find the event
  return false;
}

bool IceEventManager::Fire(u16 _eCode, void* _sender, IceEventData _data)
{
  if (state[_eCode].registeredEvents.size() == 0)
  {
    // Warn about firing empty event
    return false; // true?
  }

  for (const auto& e : state[_eCode].registeredEvents)
  {
    if (!e.callback(_eCode, _sender, e.listener, _data))
    {
      // Failed to execute the callback
      // Not necessarily fatal
    }
  }

  return true;
}
