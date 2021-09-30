
#include "asserts.h"
#include "logger.h"

#include "core/event.h"
#include "core/memory_manager.h"

IceEventManager EventManager;

void IceEventManager::Initialize()
{
  LogInfo("Initialized Event system");
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
  LogInfo("Shutdown Event system");
}

bool IceEventManager::Register(u16 _eCode, void* _listener, EventCallback _callaback)
{
  // Ensure not already registered
  for (const auto& e : state[_eCode].registeredEvents)
  {
    if (e.listener == _listener && e.callback == _callaback)
    {
      // Warn of duplicate register attempt
      ICE_ASSERT_MSG(e.listener != _listener && e.callback != _callaback,
                     "Event already registered");
      return false;
    }
  }

  RegisteredEvent re  = {_listener, _callaback};
  state[_eCode].registeredEvents.push_back(re);

  return true;
}

bool IceEventManager::Unregister(u16 _eCode, void* _listener, EventCallback _callaback)
{
  size_t size = state[_eCode].registeredEvents.size();
  // Locate the desired registered event
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
      LogError("Failed to execute a %su event", _eCode);
    }
  }

  return true;
}
