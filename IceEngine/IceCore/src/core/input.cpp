
#include "logger.h"

#include "core/input.h"
#include "core/event.h"
#include "core/memory_manager.h"

IceInput Input;

void IceInput::Initialize()
{
  MemoryManager.Zero(&m_states, sizeof(InputStates));
  IceLogInfo("Initialized Input system");
}

void IceInput::Shutdown()
{
  IceLogInfo("Shutdown Input system");
}

void IceInput::Update()
{
  MemoryManager.Copy(&m_states.keyboardPrevious, &m_states.keyboardCurrent, sizeof(keyboardState));
  MemoryManager.Copy(&m_states.mousePrevious, &m_states.mouseCurrent, sizeof(mouseState));
}

void IceInput::ProcessKeyboardKey(IceKeyCodeFlag _key, b8 _pressed)
{
  m_states.keyboardCurrent.keys[_key] = _pressed;

  //IceEventData data;
  //data.u32[0] = _key;
  //if (_pressed)
  //{
  //  EventManager.Fire(Ice_Event_Key_Pressed, nullptr, data);
  //}
  //else
  //{
  //  EventManager.Fire(Ice_Event_Key_Released, nullptr, data);
  //}
}

b8 IceInput::IsKeyDown(IceKeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key];
}

b8 IceInput::WasKeyDown(IceKeyCodeFlag _key)
{
  return m_states.keyboardPrevious.keys[_key];
}

b8 IceInput::OnKeyPressed(IceKeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key] && !m_states.keyboardPrevious.keys[_key];
}

b8 IceInput::OnKeyReleased(IceKeyCodeFlag _key)
{
  return !m_states.keyboardCurrent.keys[_key] && m_states.keyboardPrevious.keys[_key];
}

b8 IceInput::OnKeyHold(IceKeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key] && m_states.keyboardPrevious.keys[_key];
}

void IceInput::ProcessMouseButton(IceMouseButtonFlag _button, b8 _pressed)
{
  m_states.mouseCurrent.buttons[_button] = _pressed;

  //IceEventData data;
  //data.u32[0] = _button;
  //if (_pressed)
  //{
  //  EventManager.Fire(Ice_Event_Mouse_Button_Pressed, nullptr, data);
  //}
  //else
  //{
  //  EventManager.Fire(Ice_Event_Mouse_Button_Released, nullptr, data);
  //}
}

void IceInput::ProcessMouseMove(i32 _x, i32 _y)
{
  if (m_states.mouseCurrent.x != _x || m_states.mouseCurrent.y != _y)
  {
    m_states.mouseCurrent.x = _x;
    m_states.mouseCurrent.y = _y;

    //i32 deltaX = 0, deltaY = 0;
    //GetMouseDelta(&deltaX, &deltaY);

    //IceEventData data;
    //data.u32[0] = *((u32*)&deltaX);
    //data.u32[1] = *((u32*)&deltaY);
    //EventManager.Fire(Ice_Event_Mouse_Moved, nullptr, data);
  }
}

void IceInput::ProcessMouseWheel(i32 _magnitude)
{
  m_states.mouseCurrent.wheel = _magnitude;
}

b8 IceInput::IsMouseButtonDown(IceMouseButtonFlag _button)
{
  return m_states.mouseCurrent.buttons[_button];
}

b8 IceInput::WasMouseButtonDown(IceMouseButtonFlag _button)
{
  return m_states.mousePrevious.buttons[_button];
}

void IceInput::GetMousePosition(i32* _x, i32* _y)
{
  *_x = m_states.mouseCurrent.x;
  *_y = m_states.mouseCurrent.y;
}

void IceInput::GetMousePreviousPosition(i32* _x, i32* _y)
{
  *_x = m_states.mousePrevious.x;
  *_y = m_states.mousePrevious.y;
}

void IceInput::GetMouseDelta(i32* _x, i32* _y)
{
  *_x = m_states.mouseCurrent.x - m_states.mousePrevious.x;
  *_y = m_states.mouseCurrent.y - m_states.mousePrevious.y;
}

