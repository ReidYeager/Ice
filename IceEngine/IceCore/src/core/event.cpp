
#include "logger.h"
#include "core/event.h"
#include "core/memory_manager.h"

IceEventManager EventManager;

void IceEventManager::Initialize()
{
  MemoryManager.Zero(state, sizeof(CodeEvents) * Ice_Event_Max);
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
  for (u32 i = 0; i < state[_eCode].registeredEvents.size(); i++)
  {
    const auto& e = state[_eCode].registeredEvents[i];
    if (e.listener == _listener && e.callback == _callaback)
    {
      RegisteredEvent back = state[_eCode].registeredEvents.back();
      state[_eCode].registeredEvents.pop_back();

      // If not the final event, replace it
      if (i != state[_eCode].registeredEvents.size())
      {
        state[_eCode].registeredEvents[i] = back;
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
    if (!e.callback(_eCode, _sender, e.listener, {}))
    {
      // Failed to execute the callback
      // Not necessarily fatal
    }
  }

  return true;
}
