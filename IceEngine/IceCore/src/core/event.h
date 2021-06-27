
#ifndef CORE_EVENT_H
#define CORE_EVENT_H 1

#include "defines.h"
#include <vector>

enum IceEventCodes
{
  Ice_Event_Unknown = 0,

  // ===== Input =====
  Ice_Event_Key_Pressed,
  Ice_Event_Key_Released,
  Ice_Event_Mouse_Button_Pressed,
  Ice_Event_Mouse_Button_Released,
  Ice_Event_Mouse_Moved,
  Ice_Event_Mouse_Wheel,

  // ===== Window / Platform =====
  Ice_Event_Quit,
  Ice_Event_Window_Resized,

  Ice_Event_Max = 0xFF
};

struct IceEventData
{
  union data
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
};

// Return true if handled
typedef bool (*EventCallback)(u16 _eventCode, void* _sender, void* _listener, IceEventData _data);

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
