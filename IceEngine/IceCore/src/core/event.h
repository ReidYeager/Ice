
#ifndef ICE_CORE_EVENT_H_
#define ICE_CORE_EVENT_H_

#include "defines.h"
#include <vector>

enum IceEventCodes
{
  Ice_Event_Unknown = 0,

  // ===== Input =====

  // u32[0] : The pressed keycode
  Ice_Event_Key_Pressed,
  // u32[0] : The released keycode
  Ice_Event_Key_Released,
  // u32[0] : The button pressed
  Ice_Event_Mouse_Button_Pressed,
  // u32[0] : The button released
  Ice_Event_Mouse_Button_Released,
  // i32[0] : The x-axis delta
  // i32[1] : The y-axis delta
  Ice_Event_Mouse_Moved,

  Ice_Event_Mouse_Wheel,

  // ===== Window / Platform =====
  Ice_Event_Quit,
  // u32[0] : The new x-axis size
  // u32[1] : The new y-axis size
  Ice_Event_Window_Resized,

  Ice_Event_Max = 0xFF
};

struct IceEventData
{
  i64 i64[2];
  i32 i32[4];
  i16 i16[8];
  i8  i8[16];

  u64 u64[2];
  u32 u32[4];
  u16 u16[8];
  u8  u8[16];

  f64 f64[2];
  f32 f32[4];

  char c[16];
};

// Return true if handled
typedef bool(*EventCallback)(u16 _eventCode, void* _sender, void* _listener, IceEventData _data);

class IceEventManager
{
private:
  struct RegisteredEvent
  {
    void* listener;
    EventCallback callback;
  };

  struct CodeEvents
  {
    std::vector<RegisteredEvent> registeredEvents;
  };

  //RegisteredCodes[Ice_Event_Max];
  CodeEvents state[Ice_Event_Max];

public:
  void Initialize();
  void Shutdown();

  bool Register(u16 _code, void* _listener, EventCallback _callaback);
  bool Unregister(u16 _code, void* _listener, EventCallback _callaback);

  bool Fire(u16 _code, void* _sender, IceEventData _data);

};

extern IceEventManager EventManager;

#endif // !CORE_EVENT_H
